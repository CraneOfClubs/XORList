#pragma once
#include <memory> 
#include <cstdint>
#include <algorithm> 
#include "Utils.h"


template <typename T, class TAllocator = std::allocator<T>>
class LinkedList
{
public:
 //Почему не биндится напрямую?
	class Node;

	LinkedList() : LinkedList(TAllocator()) {
	}

	explicit LinkedList(const TAllocator &alloc) : allocator(alloc), nHead(&nTail), nTail(&nHead) {
	}

private:
	//friend class Utils;
	struct Node {
		uintptr_t xor_pointer;
		T value;
		explicit Node(Node *xor_pointer = nullptr) : xor_pointer(reinterpret_cast<uintptr_t>(xor_pointer)) {
		}

		template<typename... Args>
		Node(Args&&... args) : value(::std::forward<Args>(args)...){
		}
		Node(const Node&) noexcept = default;
		Node(Node &&) noexcept = default;
		virtual ~Node() = default;
		Node& operator=(const Node&) noexcept = default;
		Node& operator=(Node &&) noexcept = default;
	};


	using RebindAlloc = typename std::allocator_traits<TAllocator>::template rebind_alloc<Node>;

	RebindAlloc allocator;

	template<typename Iter, typename V>
	class Iterator {

	public:
		V& operator*() const {
			return static_cast<Node*>(current)->value;
		}

		Iter& operator++() {
			Node *next = reinterpret_cast<Node*>(xorPointers(previous, current->xor_pointer));
			previous = current;
			current = next;
			return static_cast<Iter&>(*this);
		}

		bool operator==(const Iter &right) const noexcept {
			return (current == right.current);
		}

		bool operator!=(const Iter &right) const noexcept {
			return !(*this == right);
		}

		V* operator->() const {
			return ::std::addressof(static_cast<NodeWithValue*>(current)->value);
		}

		Iter operator++(int) {
			Iter result(static_cast<Iter&>(*this));
			(void)++(*this);
			return result;
		}

		Iter& operator--() {
			Node *newPrev = reinterpret_cast<Node*>(_xor(previous->xor_pointer, current));
			current = previous;
			previous = newPrev;
			return static_cast<Iter&>(*this);
		}

		Iter operator--(int) {
			Iter result(static_cast<Iter&>(*this));
			(void)--(*this);
			return result;
		}

	protected:
		Node *previous;
		Node *current;
		Iterator(Node *previous = nullptr, Node *current = nullptr) noexcept : previous(previous), current(current) {
		}
		Iterator(const Iterator&) noexcept = default;
		Iterator(Iterator&&) noexcept = default;
		~Iterator() noexcept = default;
		Iterator& operator=(const Iterator&) noexcept = default;
		Iterator& operator=(Iterator&&) noexcept = default;

	private:
		friend class LinkedList<T, TAllocator>;
	};

	class iterator : public Iterator<iterator, T>
	{
		class const_iterator;
	public:
		iterator() noexcept = default;
		iterator(const iterator&) noexcept = default;
		iterator(iterator&&) noexcept = default;
		~iterator() noexcept = default;
		iterator& operator=(const iterator&) noexcept = default;
		iterator& operator=(iterator&&) noexcept = default;
		operator const_iterator() const noexcept {
			return { this->prev, this->current };
		}

	private:
		friend class LinkedList<T, TAllocator>;
		friend class const_iterator;
		iterator(Node *prev, Node *current) noexcept : Iterator<iterator, T>(prev, current) {
		}
	};

	class const_iterator : public Iterator<const_iterator, const T>
	{
	public:
		const_iterator() noexcept = default;
		const_iterator(const const_iterator&) noexcept = default;
		const_iterator(const_iterator&&) noexcept = default;
		~const_iterator() noexcept = default;
		const_iterator& operator=(const const_iterator&) noexcept = default;
		const_iterator& operator=(const_iterator&&) noexcept = default;
		explicit operator iterator() const noexcept {
			return { this->prev, this->current };
		}
	private:
		friend class LinkedList<T, TAllocator>;
		friend class iterator;
		const_iterator(Node *previous, Node *current) noexcept : Iterator<const_iterator, const T>(previous, current) {
		}
	};
	mutable Node nHead;
	mutable Node nTail;
	std::size_t _size = 0;

	template<typename... Args>
	Node* createNode(Args&&... args) {
		Node *const result = std::allocator_traits<RebindAlloc>::allocate(allocator, 1);

		try {
			allocator.construct(result, std::forward<Args>(args)...);
		}
		catch (...) {
			allocator.deallocate(result, 1);
			throw;
		}
		return result;
	}

	static uintptr_t _xor(const uintptr_t first, const uintptr_t second) noexcept {
		return first ^ second;
	}

	static uintptr_t _xor(Node *const first, Node *const second) noexcept {
		return _xor(reinterpret_cast<const uintptr_t>(first), reinterpret_cast<const uintptr_t>(second));
	}

	static uintptr_t _xor(const uintptr_t first, Node *const second) noexcept {
		return _xor(second, first);
	}

	static uintptr_t _xor(Node *const first, const uintptr_t second) noexcept {
		return _xor(reinterpret_cast<const uintptr_t>(first), second);
	}


	static std::pair<iterator, iterator> _insertPrev(const_iterator position, Node *const node) noexcept{
		node->xor_pointer = _xor(position.previous, position.current);
		if (position.previous != nullptr) {
			position.previous->xor_pointer = _xor(_xor(position.previous->xor_pointer, position.current), node);
		}
		if (position.current != nullptr) {
			position.current->xor_pointer = _xor(_xor(position.current->xor_pointer, position.previous), node);
		}
		return { { position.previous, node },{ node, position.current } };
	}


	template <typename... Args>
	void _push_back(Args&&... args) {
		_insertPrev(cend(), createNode(std::forward<Args>(args)...));
	}

	template <typename... Args>
	void _push_front(Args&&... args) {
		_insertPrev(cbegin(), createNode(std::forward<Args>(args)...));
	}

	public:
		//Public contract;

	template <typename... Args>
	void emplace_back(Args&&... args) {
		_push_back(args);
	}

	template <typename... Args>
	void emplace_front(Args&&... args) {
		_push_front(args);
	}

	T& front() noexcept {
		return *begin();
	}

	T& back() noexcept {
		return *(--end());
	}

	const T& back() const noexcept {
		return *(--cend());
	}

	const T& front() const noexcept {
		return *cbegin();
	}

	void push_back(const T& data) {
		_push_back(data);
	}

	void push_front(const T& data) {
		_push_front(data);
	}

	void push_back(T &&data) {
		_push_back(std::move(data));
	}

	void push_front(T &&data) {
		_push_front(std::move(data));
	}

	iterator begin() noexcept {
		return { &nHead, reinterpret_cast<Node*>(nHead.xor_pointer) };
	}

	iterator end() noexcept {
		return { reinterpret_cast<Node*>(nTail.xor_pointer), &nTail };
	}

	const_iterator begin() const noexcept {
		return cbegin();
	}

	const_iterator end() const noexcept {
		return cend();
	}

	const_iterator cbegin() const noexcept {
		return { &nHead, reinterpret_cast<Node*>(nHead.xor_pointer) };
	}

	const_iterator cend() const noexcept {
		return { reinterpret_cast<Node*>(nTail.xor_pointer), &nTail };
	}
};