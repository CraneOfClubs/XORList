#pragma once
#include <memory> 
#include <cstdint>
#include <algorithm> 


template <typename T, class TAllocator = std::allocator<T>>
class LinkedList
{
public:
	using RebindAlloc = typename std::allocator_traits<TAllocator>::template rebind_alloc<T>; //Почему не биндится напрямую?
	class Node;

	RebindAlloc allocator;
	LinkedList() : LinkedList(TAllocator()) {
	}

	explicit LinkedList(const TAllocator &alloc) : allocator(alloc), nHead(&nTail), nTail(&nHead) {
	}

private:
	struct Node {
		uintptr_t xor_pointer;
		T value;
		explicit Node(Node *xorPtr = nullptr) : xor_pointer(reinterpret_cast<uintptr_t>(xorPtr)) {
		}
		Node(const Node&) noexcept = default;
		Node(Node &&) noexcept = default;
		virtual ~Node() = default;
		Node& operator=(const Node&) noexcept = default;
		Node& operator=(Node &&) noexcept = default;
	};

	template<typename Iter, typename V>
	class Iterator {

	public:
		V& operator*() const {
			return static_cast<NodeWithValue*>(current)->value;
		}

		Iter& operator++() {
			Node *next = reinterpret_cast<Node*>(xorPointers(previous, current->xorPtr));
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
			Node *newPrev = reinterpret_cast<Node*>(xorPointers(previous->xorPtr, current));
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
		Iterator(Node *prev = nullptr, Node *current = nullptr) noexcept : previous(prev), current(current) {
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
		const_iterator(Node *prev, Node *current) noexcept : IteratorBase<const_iterator, const T>(prev, current) {
		}
	};
	mutable Node nHead;
	mutable Node nTail;

	public:
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
		return { &nHead, reinterpret_cast<Node*>(nHead.xorPtr) };
	}

	const_iterator cend() const noexcept {
		return { reinterpret_cast<Node*>(nTail.xorPtr), &nTail };
	}
};