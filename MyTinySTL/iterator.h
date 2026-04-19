#ifndef MYTINYSTL_ITERATOR_H_
#define MYTINYSTL_ITERATOR_H_

#include <cstddef>

#include "type_traits.h"

namespace mystl {
	// 五种迭代器类型
	struct input_iterator_tag {};
	struct output_iterator_tag {};
	struct forward_iterator_tag : public input_iterator_tag {};
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};

	// 迭代器模板
	template <class Category, class T, class Distance = ptrdiff_t, class Pointer = T*, class Reference = T&>
	struct iterator {
		typedef Category  iterator_category;
		typedef T         value_type;
		typedef Distance  difference_type;
		typedef Pointer   pointer;
		typedef Reference reference;
	};

	// 判断一个类型T是否为迭代器类型
	template <class T>
	struct has_iterator_cat {
	private:
		struct two {
			char a;
			char b;
		};
		template <class U> static two test(...);
		template <class U> static char test(typename U::iterator_category* = 0);
	public:
		static const bool value = sizeof(test<T>(0)) == sizeof(char);
	};

	// 提取迭代器的五个参数
	// 主模板
	template <class Iterator, bool>
	struct iterator_traits_impl {};

	// 偏特化版本
	template <class Iterator>
	struct iterator_traits_impl<Iterator, true> {
		typedef typename Iterator::iterator_category iterator_category;
		typedef typename Iterator::value_type        value_type;
		typedef typename Iterator::difference_type   difference_type;
		typedef typename Iterator::pointer           pointer;
		typedef typename Iterator::reference         reference;
	};

	/*
	检查一个类是不是合法迭代器
	它和has_iterator_cat的区别是，has_iterator_cat只检查是否有iterator_category这个成员类型
	而iterator_traits_helper还检查了iterator_category是否是五种迭代器类型之一
	*/
	template <class Iterator, bool>
	struct iterator_traits_helper {};

	template <class Iterator>
	struct iterator_traits_helper<Iterator, true> : public iterator_traits_impl<Iterator,
	std::is_convertible<typename Iterator::iterator_category, input_iterator_tag>::value ||
	std::is_convertible<typename Iterator::iterator_category, output_iterator_tag>::value> {};

	// 迭代器萃取器
	template <class Iterator>
	struct iterator_traits : public iterator_traits_helper<Iterator, has_iterator_cat<Iterator>::value> {};

	// 偏特化版本
	template <class T>
	struct iterator_traits<T*> {
		typedef random_access_iterator_tag iterator_category;
		typedef T                          value_type;
		typedef ptrdiff_t                  difference_type;
		typedef T*                         pointer;
		typedef T&                         reference;
	};

	template <class T>
	struct iterator_traits<const T*> {
		typedef random_access_iterator_tag iterator_category;
		typedef T                          value_type;
		typedef ptrdiff_t                  difference_type;
		typedef const T*                   pointer;
		typedef const T&                   reference;
	};

	// 确定type_traits.h中的布尔常量
	template <class T, class U, bool = has_iterator_cat<iterator_traits<T>>::value>
	struct has_iterator_cat_of : public m_bool_constant<std::is_convertible<typename iterator_traits<T>::iterator_category, U>::value> {};

	// 偏特化处理
	template <class T, class U>
	struct has_iterator_cat_of<T, U, false> : public m_false_type {};

	// 判断一个迭代器是否为只读单向迭代器
	template <class Iter>
	struct is_exactly_input_iterator : public m_bool_constant<has_iterator_cat_of<Iter, input_iterator_tag>::value &&
		!has_iterator_cat_of<Iter, forward_iterator_tag>::value> {};

	// 迭代器类型判断
	template <class Iter>
	struct is_input_iterator : public has_iterator_cat_of<Iter, input_iterator_tag> {};

	template <class Iter>
	struct is_output_iterator : public has_iterator_cat_of<Iter, output_iterator_tag> {};

	template <class Iter>
	struct is_forward_iterator : public has_iterator_cat_of<Iter, forward_iterator_tag> {};

	template <class Iter>
	struct is_bidirectional_iterator : public has_iterator_cat_of<Iter, bidirectional_iterator_tag> {};

	template <class Iter>
	struct is_random_access_iterator : public has_iterator_cat_of<Iter, random_access_iterator_tag> {};

	template <class Iterator>
	struct is_iterator : public m_bool_constant<is_input_iterator<Iterator>::value || is_output_iterator<Iterator>::value> {};

	// 判断迭代器三个参数的类型
	template <class Iterator>
	typename iterator_traits<Iterator>::iterator_category
		iterator_category(const Iterator&) {
		typedef typename iterator_traits<Iterator>::iterator_category Category;
		return Category();
	}

	template <class Iterator>
	typename iterator_traits<Iterator>::value_type*
		value_type(const Iterator&) {
		return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
	}

	template <class Iterator>
	typename iterator_traits<Iterator>::difference_type*
		distance_type(const Iterator&) {
		return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
	};

	// 迭代器距离计算
	// 只读单向迭代器
	template <class InputIterator>
	typename iterator_traits<InputIterator>::difference_type
		distance_dispatch(InputIterator first, InputIterator last, input_iterator_tag) {
		typename iterator_traits<InputIterator>::difference_type n = 0;
		while (first != last) {
			++first;
			++n;
		}
		return n;
	}

	// 随机访问迭代器
	template <class RandomIter>
	typename iterator_traits<RandomIter>::difference_type
		distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag) {
		return last - first;
	}

	template <class InputIterator>
	typename iterator_traits<InputIterator>::difference_type
		distance(InputIterator first, InputIterator last) {
		return distance_dispatch(first, last, iterator_category(first));
	}

	// 迭代器前进
	// 单向迭代器
	template <class InputIterator, class Distance>
	void advance_dispatch(InputIterator& i, Distance n, input_iterator_tag) {
		while (n--) {
			++i;
		}
	}

	// 双向迭代器
	template <class BidirectionalIterator, class Distance>
	void advance_dispatch(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag) {
		if (n >= 0)
			while (n--) ++i;
		else
			while (n++) --i;
	}

	// 随机访问迭代器
	template <class RandomIter, class Distance>
	void advance_dispatch(RandomIter& i, Distance n, random_access_iterator_tag) {
		i += n;
	}

	template <class InputIterator, class Distance>
	void advance(InputIterator& i, Distance n) {
		advance_dispatch(i, n, iterator_category(i));
	}

	// 反向迭代器
	template <class Iterator>
	class reverse_iterator {
	private:
		// 对应的正向迭代器
		Iterator current;
	public:
		// 五种类型，一个不少
		typedef typename iterator_traits<Iterator>::iterator_category iterator_category;
		typedef typename iterator_traits<Iterator>::value_type        value_type;
		typedef typename iterator_traits<Iterator>::difference_type   difference_type;
		typedef typename iterator_traits<Iterator>::pointer           pointer;
		typedef typename iterator_traits<Iterator>::reference         reference;
		// 起名，方便使用
		typedef Iterator											  iterator_type;
		typedef reverse_iterator<Iterator>							  self;

		// 构造函数
		reverse_iterator() : current() {}
		explicit reverse_iterator(iterator_type x) : current(x) {}
		reverse_iterator(const self& x) : current(x.current) {}

		// 取出对应的正向迭代器
		iterator_type base() const {
			return current;
		}

		// 重载解引用运算符*
		reference operator*() const {
			auto tmp = current;
			return *--tmp;
		}
		// 重载箭头运算符->
		pointer operator->() const {
			return &(operator*());
		}
		// 重载前置递增运算符++
		self& operator++() {
			--current;
			return *this;
		}
		// 重载后置递增运算符++
		self operator++(int) {
			self tmp = *this;
			--current;
			return tmp;
		}
		// 重载前置递减运算符--
		self& operator--() {
			++current;
			return *this;
		}
		// 重载后置递减运算符--
		self operator--(int) {
			self tmp = *this;
			++current;
			return tmp;
		}
		// 重载加法运算符+
		self operator+(difference_type n) const {
			return self(current - n);
		}
		// 重载减法运算符-
		self operator-(difference_type n) const {
			return self(current + n);
		}
		// 重载复合赋值运算符+=
		self& operator+=(difference_type n) {
			current -= n;
			return *this;
		}
		// 重载复合赋值运算符-=
		self& operator-=(difference_type n) {
			current += n;
			return *this;
		}
		// 重载下标运算符[]
		reference operator[](difference_type n) const {
			return *(*this + n);
		}
	};

	// 反向迭代器的比较运算符
	template <class Iterator>
	bool operator==(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
		return lhs.base() == rhs.base();
	}
	// 重载不等于运算符!=
	template <class Iterator>
	bool operator!=(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
		return !(lhs == rhs);
	}
	// 重载小于运算符<
	template <class Iterator>
	bool operator<(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
		return rhs.base() < lhs.base();
	}
	// 重载小于等于运算符<=
	template <class Iterator>
	bool operator<=(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
		return !(rhs < lhs);
	}
	// 重载大于运算符>
	template <class Iterator>
	bool operator>(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
		return rhs < lhs;
	}
	// 重载大于等于运算符>=
	template <class Iterator>
	bool operator>=(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
		return !(lhs < rhs);
	}

	// 重载减法运算符-
	template <class Iterator>
	typename reverse_iterator<Iterator>::difference_type
		operator-(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
		return rhs.base() - lhs.base();
	}
}

#endif