#pragma once
#include <memory> 
#include <cstdint>
#include <algorithm> 
#include <iterator>
#include <tuple>
#include <array>
#include <initializer_list>


template <typename T, class TAllocator = std::allocator<T>>
class LinkedList
{
public:
 //ѕочему не биндитс€ напр€мую?

	template<typename Iter, typename V>
	class Iterator;
	struct Node;
	typedef T value_type;

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


	LinkedList() : LinkedList(TAllocator()) {
	}

	explicit LinkedList(const TAllocator &alloc) : allocator(alloc), nHead(&nTail), nTail(&nHead) {
	}

	LinkedList(std::initializer_list<T> il, const TAllocator &alloc = TAllocator()) : LinkedList(alloc) {
		assign(std::move(il));
	}

	explicit LinkedList(std::size_t n, const TAllocator &alloc = TAllocator()): LinkedList(alloc) {
		resize(n);
	}

	LinkedList(std::size_t n, const T& val, const TAllocator &alloc = TAllocator()) : LinkedList(alloc){
		resize(n, val);
	}

	LinkedList(const LinkedList &other)
		: LinkedList(::std::allocator_traits<TAllocator>::select_on_container_copy_construction(other.allocator)) {
		insert(cbegin(), other.cbegin(), other.cend());
	}

	LinkedList(LinkedList &&other) : allocator(std::move(other.allocator)), nHead(&nTail), nTail(&nHead) {
		if (!other.empty())
			splice(cbegin(), other);
	}

	virtual ~LinkedList(){
		clear();
	}

	LinkedList& operator=(const LinkedList &right){
		if (this != ::std::addressof(right))
			_copyAssignment(right);
		return *this;
	}

	LinkedList& operator=(LinkedList &&right){
		if (this != ::std::addressof(right))
			_moveAssignment(std::move(right));
		return *this;
	}

	
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



#ifdef DEBUG_TEST
	int amount_of_copy = 0;
#endif // DEBUG_TEST

private:
	//friend class Utils;
	struct Node {
		uintptr_t xor_pointer;
		T value;
		explicit Node(Node *xor_pointer = nullptr) : xor_pointer(reinterpret_cast<uintptr_t>(xor_pointer)) {
		}

		template<typename... Args>
		Node(Args&&... args) : value(std::forward<Args>(args)...){
		}
#ifdef DEBUG_TEST

		Node(const Node& other) noexcept {
			xor_pointer = other.xor_pointer;
			value = other.value;
			amount_of_copy++;
		}
#else
		Node(const Node&) noexcept = default;
#endif // DEBUG_TEST


		Node(Node &&) noexcept = default;
		virtual ~Node() = default;
		Node& operator=(const Node&) noexcept = default;
		Node& operator=(Node &&) noexcept = default;
	};

	struct Block {
		std::pair<iterator, iterator> block;
		iterator end;

		Block(iterator first, iterator last, iterator end) : block(first, last), end(end) {
		}
	};


	using RebindAlloc = typename std::allocator_traits<TAllocator>::template rebind_alloc<Node>;

	RebindAlloc allocator;
	using difference_type = std::ptrdiff_t;
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

	template<typename... Args>
	void _resize(std::size_t count, Args&&... args) {
		if (count == 0) {
			clear();
			return;
		}

		while (size() > count)
			pop_back();

		while (size() < count)
			emplace_back(std::forward<Args>(args)...);
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

	static std::pair<iterator, iterator> _insertBlockPrev(const_iterator position, const_iterator begin, const_iterator end) noexcept {
		begin.current->xor_pointer = _xor(_xor(begin.current->xor_pointer, begin.previous), position.previous);
		end.previous->xor_pointer = _xor(_xor(end.previous->xor_pointer, end.current), position.current);

		if (position.previous != nullptr) {
			position.previous->xor_pointer = _xor(_xor(position.previous->xor_pointer, position.current), begin.current);
		}
		if (position.current != nullptr) {
			position.current->xor_pointer = _xor(_xor(position.current->xor_pointer, position.previous), end.previous);
		}

		return { { position.previous, begin.current },{ end.previous, position.current } };
	}

	template<typename LessCompare>
	static std::pair<iterator, iterator> mergeSequences(const_iterator beginTo, const_iterator endTo, const_iterator beginFrom, 
		const_iterator endFrom, LessCompare &&isLess) noexcept {
		const_iterator resultBegin = beginTo;

		while (beginFrom != endFrom) {
			if (beginTo == endTo) {
				if (resultBegin == beginTo) {
					std::tie(resultBegin, endTo) = _insertBlockPrev(beginTo, beginFrom, endFrom);
				}
				else {
					endTo = _insertBlockPrev(beginTo, beginFrom, endFrom).second;
				}

				break;
			}
			else if (std::forward<LessCompare>(isLess)(*beginFrom, *beginTo)) {
				auto cutResult = _deleteBlock(beginFrom, std::next(beginFrom));

				if (resultBegin == beginTo) {
					std::tie(resultBegin, beginTo) = _insertPrev(beginTo, static_cast<Node *>(cutResult.block.first.current));
				}
				else {
					beginTo = _insertPrev(beginTo, static_cast<Node*>(cutResult.block.first.current)).second;
				}

				beginFrom = cutResult.end;
			}
			else {
				++beginTo;
			}
		}

		return { static_cast<iterator>(resultBegin), static_cast<iterator>(endTo) };
	}

	//Local private
	template <typename... Args>
	void _push_back(Args&&... args) {
		_insertPrevHere(cend(), createNode(std::forward<Args>(args)...));
	}

	template <typename... Args>
	void _push_front(Args&&... args) {
		_insertPrevHere(cbegin(), createNode(std::forward<Args>(args)...));
	}

	void swapWithoutAllocators(LinkedList &other) {
		const auto thisDistance = size();
		const auto otherDistance = other.size();
		if (!empty()) {
			auto thisBlock = _deleteBlockHere(cbegin(), cend(), thisDistance);

			if (!other.empty()) {
				auto otherBlock = other._deleteBlockHere(other.cbegin(), other.cend(), otherDistance);
				_insertBlockPrevHere(cbegin(), otherBlock.block.first, otherBlock.block.second, otherDistance);
			}

			other._insertBlockPrevHere(other.cbegin(), thisBlock.block.first, thisBlock.block.second, thisDistance);
		}
		else if (!other.empty())
		{
			auto otherCutResult = other._deleteBlockHere(other.cbegin(), other.cend(), otherDistance);
			_insertBlockPrevHere(cbegin(), otherCutResult.block.first, otherCutResult.block.second, otherDistance);
		}
	}

	public:
		//“ест под сплайс чтобы не было копировани€.
		// онстантные итераторы
		// онструкторы с итераторами и конструкторы с интами (констекспр).
	//Public contract -----------------------------------------------------------

	template <typename... Args>
	void emplace_back(Args&&... args) {
		int t = 0;
		_insertPrevHere(cend(), createNode(::std::forward<Args>(args)...));
	}

	template <typename... Args>
	void emplace_front(Args&&... args) {
		_insertPrevHere(cbegin(), createNode(::std::forward<Args>(args)...));
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

	void resize(std::size_t count){
		_resize(count);
	}

	void resize(std::size_t count, const T& val) {
		_resize(count, val);
	}

	void assign(std::initializer_list<T> il) {
		auto test = il.begin();
		auto test2 = il.end();
		assign(il.begin(), il.end());
	}

	void swap(LinkedList &other) {
		_swap(other);
	}

	void assign(std::size_t count, const T& val) {
		auto iter = begin();
		for (; (iter != end()) && (count > 0); ++iter, --count)
			*iter = val;

		if (iter != end())
			erase(iter, end());
		else {
			while (count > 0) {
				emplace_back(val);
				--count;
			}
		}
	}

	template <typename InputIterator>
	typename ::std::enable_if<::std::is_base_of<::std::input_iterator_tag, typename ::std::iterator_traits<InputIterator>::iterator_category>::value>::type
				assign(InputIterator first, InputIterator last)
	{
		for (auto iter = begin(); iter != end(); ++iter, ++first) {
			if (first == last) {
				erase(iter, end());
				return;
			}
			*iter = *first;
		}

		for (; first != last; ++first) {
			emplace_back(*first);
		}
	}

	template<typename Alloc = RebindAlloc>
	typename std::enable_if<std::allocator_traits<Alloc>::propagate_on_container_swap::value>::type _swap(LinkedList &other) {
		std::swap(allocator, other.allocator);
		swapWithoutAllocators(other);
	}

	template<typename Alloc = RebindAlloc>
	typename std::enable_if<!std::allocator_traits<Alloc>::propagate_on_container_swap::value>::type _swap(LinkedList &other) {
		swapWithoutAllocators(other);
	}

	void sort() noexcept
	{
		sort(std::less<T>{});
	}

	template <class Compare>
	void sort(Compare comp) noexcept
	{
		using Range = std::pair<const_iterator, const_iterator>;
		struct NullableRange {
			Range range;
			bool isNull = true;
		};
		const auto thisSize = size();

		if (thisSize < 2)
			return;

		std::array<NullableRange, 64> sortedRanges;
		while (!empty()) {
			Range newRange = _deleteBlockHere(cbegin(), ++cbegin(), 1).block;

			std::uint32_t i = 0;
			for (; i < sortedRanges.size(); ++i) {
				if (sortedRanges[i].isNull) {
					sortedRanges[i].range = newRange;
					sortedRanges[i].isNull = false;
					break;
				}
				else {
					newRange = mergeSequences(sortedRanges[i].range.first, sortedRanges[i].range.second,
						newRange.first, newRange.second, comp);
					sortedRanges[i].isNull = true;
				}
			}

			if (i == sortedRanges.size()) {
				//sortedRanges.back().range = newRange;
				//sortedRanges.back().isNull = false;
			}
		}

		NullableRange result;
		for (std::uint32_t i = 0; i < sortedRanges.size(); ++i) {
			if (!sortedRanges[i].isNull) {
				if (result.isNull) {
					result = sortedRanges[i];
				}
				else {
					result.range = mergeSequences(sortedRanges[i].range.first, sortedRanges[i].range.second,
						result.range.first, result.range.second, comp);
				}
			}
		}
		(void)_insertBlockPrevHere(cend(), result.range.first, result.range.second, thisSize);
	}


	template<typename Alloc = RebindAlloc>
	typename std::enable_if<std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value>::type _copyAssignment(const LinkedList &rval) {
		clear();
		allocator = rval.allocator;
		assign(rval.cbegin(), rval.cend());
	}

	template<typename Alloc = RebindAlloc>
	typename std::enable_if<!std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value>::type _copyAssignment(const LinkedList &rval) {
		clear();
		assign(rval.cbegin(), rval.cend());
	}

	template<typename Alloc = RebindAlloc>
	typename std::enable_if<std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value>::type _moveAssignment(LinkedList &&rval) {
		clear();
		allocator = std::move(rval.allocator);
		splice(cbegin(), rval);
	}

	template<typename Alloc = RebindAlloc>
	typename std::enable_if<!::std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value>::type _moveAssignment(LinkedList &&rval) {
		clear();
		if (allocator == rval.allocator) {
			splice(cbegin(), rval);
		}
		else {
			for (T &moved : rval)
				emplace_back(std::move(moved));
			rval.clear();
		}
	}

	template<typename I>
	Block _deleteBlockHere(const_iterator first, const_iterator last, I distance) noexcept {
		_size -= distance;
		return _deleteBlock(first, last);
	}

	template<typename I>
	std::pair<iterator, iterator> _insertBlockPrevHere(const_iterator position, const_iterator begin, const_iterator end, I range) noexcept {
		_size += range;
		return _insertBlockPrev(position, begin, end);
	}

	template<typename LessCompare, typename I>
	std::pair<iterator, iterator> mergeSequencesHere(const_iterator beginTo, const_iterator endTo, const_iterator beginFrom, 
		const_iterator endFrom, LessCompare &&isLess, I distance) noexcept {
		_size += distance;
		return mergeSequences(beginTo, endTo, beginFrom, endFrom, std::forward<LessCompare>(isLess));
	}


	template<typename I>
	void _deleteAndFreeBlock(const_iterator begin, const_iterator end, I range) {
		if (range == 0){
			return;
		}
		std::tie(begin, end) = _deleteBlockHere(begin, end, range).block; //tuple of refs

		for (; begin != end; ) {
			for (; begin != end; ) {
				allocator.destroy(static_cast<Node*>((++begin).previous));
				allocator.deallocate(static_cast<Node*>(begin.previous), 1);
			}
		}
	}

	iterator erase(const_iterator first, const_iterator last) {
		if (first != last) {
			_deleteAndFreeBlock(first, last, std::distance(first, last));
		}
		return { first.previous, last.current };
	}

	iterator erase(const_iterator position) {
		return erase(position, std::next(position));
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

	void clear() {
		_deleteAndFreeBlock(cbegin(), cend(), size());
	}

	iterator insert(const_iterator position, const T& val)
	{
		return _insertPrevHere(position, createNode(val)).first;
	}

	template <class InputIterator>
	iterator insert(const_iterator position, InputIterator first, InputIterator last)
	{
		if (first == last)
			return static_cast<iterator>(position);

		const iterator result = insert(position, *first);
		position = result;
		std::size_t count = 1;

		for (++position; ++first != last; ++position, ++count) {
			try {
				position = insert(position, *first);
			}
			catch (...) {
				_deleteAndFreeBlock(result, position, count);
				throw;
			}
		}

		return result;
	}

	void reverse() noexcept
	{
		if (empty())
			return;

		auto first = cbegin();
		auto last = --cend();
		first.current->xor_pointer = _xor(_xor(first.current->xor_pointer, std::addressof(nHead)), std::addressof(nTail));
		last.current->xor_pointer = _xor(_xor(last.current->xor_pointer, std::addressof(nTail)), std::addressof(nHead));
		nHead.xor_pointer = reinterpret_cast<uintptr_t>(last.current);
		nTail.xor_pointer = reinterpret_cast<uintptr_t>(first.current);
	}

	void splice(const_iterator position, LinkedList &x) noexcept
	{
		if ((this == std::addressof(x)) || (x.empty()))
			return;

		const auto distance = x.size();
		const auto range = x._deleteBlockHere(x.cbegin(), x.cend(), distance).block;

		_insertBlockPrevHere(position, range.first, range.second, distance);
	}

	void splice(const_iterator position, LinkedList &x, const_iterator i) noexcept
	{
		if ((this == std::addressof(x)) && ((position == i) || (position.previous == i.current)))
			return;

		const auto range = x._deleteBlockHere(i, std::next(i), 1).block;
		_insertPrevHere(position, static_cast<Node*>(range.first.current));
	}

	void splice(const_iterator position, LinkedList &x, const_iterator first, const_iterator last) noexcept
	{
		if (first == last)
			return;

		const size_t distance = (this == std::addressof(x)) ? size() : std::distance(first, last);

		std::tie(first, last) = x._deleteBlockHere(first, last, distance).block;
		_insertBlockPrevHere(position, first, last, distance);
	}

	void unique() {
		unique(std::equal_to<T>{});
	}

	template <typename BinaryPredicate>
	void unique(BinaryPredicate isEqual) {
		if (size() < 2)
			return;

		auto current = cbegin();
		for (auto prev = current++; current != cend();) {
			if (isEqual(*prev, *current))
				current = erase(current);
			else
				prev = current++;
		}
	}

	void merge(LinkedList &x) noexcept {
		merge(x, std::less<T>{});
	}

	template <typename Compare>
	void merge(LinkedList &x, Compare isLess) noexcept {
		if (!x.empty())
		{
			const auto distance = x.size();
			const auto range = x._deleteBlockHere(x.cbegin(), x.cend(), distance).block;

			mergeSequencesHere(cbegin(), cend(), range.first, range.second, ::std::move(isLess), distance);
		}
	}
};