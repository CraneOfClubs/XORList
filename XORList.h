#pragma once
#include <memory> 
#include <cstdint>
#include <algorithm> 
#include <iterator>
#include <tuple>
#include "Utils.h"


template <typename T, class TAllocator = std::allocator<T>>
class LinkedList
{
public:
 //Почему не биндится напрямую?


	LinkedList() : LinkedList(TAllocator()) {
	}

	explicit LinkedList(const TAllocator &alloc) : allocator(alloc), nHead(&nTail), nTail(&nHead) {
	}
	template<typename Iter, typename V>
	class Iterator;
	struct Node;

	class const_iterator;

	class iterator : public Iterator<iterator, T>
	{

	public:
		iterator() noexcept = default;
		iterator(const iterator&) noexcept = default;
		iterator(iterator&&) noexcept = default;
		~iterator() noexcept = default;
		iterator& operator=(const iterator&) noexcept = default;
		iterator& operator=(iterator&&) noexcept = default;
		operator const_iterator() const noexcept {
			return { this->previous, this->current };
		}

	private:
		friend class LinkedList<T, TAllocator>;
		friend class const_iterator;
		iterator(Node *previous, Node *current) noexcept : Iterator<iterator, T>(previous, current) {
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
			return { this->previous, this->current };
		}
	private:
		friend class LinkedList<T, TAllocator>;
		friend class iterator;
		const_iterator(Node *previous, Node *current) noexcept : Iterator<const_iterator, const T>(previous, current) {
		}
	};


	template<typename Iter, typename V>
	class Iterator {

	public:

		//struct std::iterator_traits<Iterator> {
		//	typedef std::ptrdiff_t difference_type;
		//	typedef V value_type;
		//	typedef value_type* pointer;
		//	typedef value_type& reference;
		//	typedef std::bidirectional_iterator_tag iterator_category;
		//};

		//iteartor_traits, avoiding changes in std:: namespace; ???
		typedef V value_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef std::ptrdiff_t difference_type;
		typedef std::bidirectional_iterator_tag iterator_category;

		reference& operator*() const {
			return static_cast<Node*>(current)->value;
		}

		Iter& operator++() {
			Node *next = reinterpret_cast<Node*>(_xor(previous, current->xor_pointer));
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

		pointer operator->() const {
			return std::addressof(static_cast<Node*>(current)->value);
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
	using difference_type = ::std::ptrdiff_t;
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

	std::pair<iterator, iterator> _insertPrevHere(const_iterator position, Node *const node) noexcept {
		++_size;
		return _insertPrev(position, node);
	}
	//Utils -----------------------
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

	static std::tuple<iterator, iterator, iterator> _delete_block(const_iterator begin, const_iterator end) noexcept
	{
		if (begin.previous != nullptr) {
			begin.previous->xor_pointer = _xor(_xor(begin.previous->xor_pointer, begin.current), end.current);
		}
		if (end.current != nullptr)
		{
			end.current->xor_pointer = _xor(_xor(end.current->xor_pointer, end.previous), begin.previous);
		}

		begin.current->xor_pointer = _xor(_xor(begin.current->xor_pointer, begin.previous), nullptr);
		end.previous->xor_pointer = _xor(_xor(end.previous->xor_pointer, end.current), nullptr);

		return { { nullptr, begin.current },{ end.previous, nullptr },{ begin.previous, end.current } };
	}

	//Local private
	template <typename... Args>
	void _push_back(Args&&... args) {
		_insertPrev(cend(), createNode(std::forward<Args>(args)...));
	}

	template <typename... Args>
	void _push_front(Args&&... args) {
		_insertPrev(cbegin(), createNode(std::forward<Args>(args)...));
	}

	public:
	//Public contract -----------------------------------------------------------

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

	struct Block {
		std::pair<iterator, iterator> block;
		iterator end;

		Block(iterator first, iterator last, iterator end): block(first, last), end(end){
		}
	};

	template<typename I>
	Block _deleteBlockHere(const_iterator first, const_iterator last, I distance) noexcept {
		_size -= distance;
		return _deleteBlock(first, last);
	}

	static Block _deleteBlock(const_iterator begin, const_iterator end) noexcept {
		if (begin.previous != nullptr) {
			begin.previous->xor_pointer = _xor(_xor(begin.previous->xor_pointer, begin.current), end.current);
		}
		if (end.current != nullptr) {
			end.current->xor_pointer = _xor(_xor(end.current->xor_pointer, end.previous), begin.previous);
		}

		begin.current->xor_pointer = _xor(_xor(begin.current->xor_pointer, begin.previous), nullptr);
		end.previous->xor_pointer = _xor(_xor(end.previous->xor_pointer, end.current), nullptr);

		return { { nullptr, begin.current },{ end.previous, nullptr },{ begin.previous, end.current } };
	}


	template<typename I>
	void _deleteAndFreeBlock(const_iterator begin, const_iterator end, I distance)
	{
		if (distance == 0){
			return;
		}
		std::tie(begin, end) = _deleteBlockHere(begin, end, distance).block; //tuple of refs

		for (; begin != end; ) {
			for (; begin != end; ) {
				allocator.destroy(static_cast<Node*>((++begin).previous));
				allocator.deallocate(static_cast<Node*>(begin.previous), 1);
			}
		}
	}

	iterator erase(const_iterator first, const_iterator last) {
		if (first != last) {
			auto test = std::distance(first, last);
			_deleteAndFreeBlock(first, last, ::std::distance(first, last));
		}
		return { first.previous, last.current };
	}

	iterator erase(const_iterator position) {
		return erase(position, std::next(position)); //Does not change iterator;
	}

	void pop_front() {
		erase(cbegin());
	}

	void pop_back() {
		erase(--cend());
	}

	size_t size() const noexcept {
		return _size;
	}

	bool empty() const noexcept {
		return (size() == 0);
	}
};