/*
 *  Copyright 2001-2007 Internet2
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file xmltooling/util/XMLObjectChildrenList.h
 * 
 * STL-compatible container wrapper
 */

#ifndef __xmltooling_list_h__
#define __xmltooling_list_h__

#include <xmltooling/exceptions.h>
#include <xmltooling/XMLObject.h>

#include <list>

/**
 * Shorthand for an XMLObjectChildrenList wrapped around a vector
 * 
 * @param type  the type of object in the vector
 */
#define VectorOf(type) xmltooling::XMLObjectChildrenList< std::vector<type*> >

/**
 * Shorthand for an XMLObjectChildrenList wrapped around a list
 * 
 * @param type  the type of object in the list
 */
#define ListOf(type) xmltooling::XMLObjectChildrenList< std::list<type*> >

/**
 * Shorthand for an XMLObjectChildrenList wrapped around a deque
 * 
 * @param type  the type of object in the deque
 */
#define DequeOf(type) xmltooling::XMLObjectChildrenList< std::deque<type*> >

/**
 * Shorthand for an XMLObjectPairList wrapped around a vector
 * 
 * @param type1  the first type of object in the vector
 * @param type2  the second type of object in the vector
 */
#define VectorOfPairs(type1,type2) xmltooling::XMLObjectPairList< std::vector< std::pair<type1*,type2*> > >

/**
 * Shorthand for an XMLObjectPairList wrapped around a list
 * 
 * @param type1  the first type of object in the vector
 * @param type2  the second type of object in the vector
 */
#define ListOfPairs(type1,type2) xmltooling::XMLObjectPairList< std::list< std::pair<type1*,type2*> > >

/**
 * Shorthand for an XMLObjectPairList wrapped around a deque
 * 
 * @param type1  the first type of object in the vector
 * @param type2  the second type of object in the vector
 */
#define DequeOfPairs(type1,type2) xmltooling::XMLObjectPairList< std::deque< std::pair<type1*,type2*> > >

namespace xmltooling {

    // Forward reference
    template <class _Tx, class _Ty=XMLObject> class XMLObjectChildrenList;
    template <class _Tx, class _Ty=XMLObject> class XMLObjectPairList;

    /**
     * STL iterator that mediates access to an iterator over typed XML children.
     * @param _Ty   a bidrectional sequence of the subtype to iterate over
     */
    template <class _Ty>
    class XMLObjectChildrenIterator
    {
        /// @cond OFF
        typename _Ty::iterator m_iter;
        template <class _Tx, class _Tz> friend class XMLObjectChildrenList;
        template <class _Tx, class _Tz> friend class XMLObjectPairList;
    public:
        typedef typename std::iterator_traits<typename _Ty::iterator>::iterator_category iterator_category;
        typedef typename std::iterator_traits<typename _Ty::iterator>::value_type value_type;
        typedef typename std::iterator_traits<typename _Ty::iterator>::difference_type difference_type;
        typedef typename std::iterator_traits<typename _Ty::iterator>::pointer pointer;
        typedef typename std::iterator_traits<typename _Ty::iterator>::reference reference;
        typedef typename _Ty::const_reference const_reference;
        typedef typename _Ty::const_pointer const_pointer;

        XMLObjectChildrenIterator() {
        }

        XMLObjectChildrenIterator(typename _Ty::iterator iter) {
            m_iter=iter;
        }

        const_reference operator*() const {
            return *m_iter;
        }

        const_reference operator->() const {
            return *m_iter;
        }

        XMLObjectChildrenIterator& operator++() {
            // preincrement
            ++m_iter;
            return (*this);
        }

        XMLObjectChildrenIterator& operator--() {
            // predecrement
            --m_iter;
            return (*this);
        }

        XMLObjectChildrenIterator operator++(int) {
            // postincrement
            XMLObjectChildrenIterator _Tmp = *this;
            ++*this;
            return (_Tmp);
        }

        XMLObjectChildrenIterator operator--(int) {
            // postdecrement
            XMLObjectChildrenIterator _Tmp = *this;
            --*this;
            return (_Tmp);
        }

        XMLObjectChildrenIterator& operator+=(difference_type _Off) {
            // increment by integer
            m_iter += _Off;
            return (*this);
        }

        XMLObjectChildrenIterator operator+(difference_type _Off) const {
            // return this + integer
            return m_iter + _Off;
        }

        XMLObjectChildrenIterator& operator-=(difference_type _Off) {
            // decrement by integer
            return (*this += -_Off);
        }

        XMLObjectChildrenIterator operator-(difference_type _Off) const {
            // return this - integer
            XMLObjectChildrenIterator _Tmp = *this;
            return (_Tmp -= _Off);
        }

        difference_type operator-(const XMLObjectChildrenIterator& _Right) const {
            // return difference of iterators
            return m_iter - _Right.m_iter;
        }

        const_reference operator[](difference_type _Off) const {
            // subscript
            return (*(*this + _Off));
        }

        bool operator==(const XMLObjectChildrenIterator &_Right) const {
		    // test for iterator equality
		    return (m_iter == _Right.m_iter);
	    }

	    bool operator!=(const XMLObjectChildrenIterator &_Right) const {
		    // test for iterator inequality
		    return (!(m_iter == _Right.m_iter));
	    }
	    
	    bool operator<(const XMLObjectChildrenIterator &_Right) const {
	        return (m_iter < _Right.m_iter);
	    }
        /// @endcond
    };

    /**
     * STL-compatible container that mediates access to underlying lists of typed XML children.
     * @param _Tx   the subtype container to encapsulate
     * @param _Ty   the base type in the underlying list (defaults to XMLObject)
     */
    template <class Container, class _Ty>
    class XMLObjectChildrenList
    {
        Container& m_container;
        typename std::list<_Ty*>* m_list;
        typename std::list<_Ty*>::iterator m_fence;
        XMLObject* m_parent;

	public:
        /// @cond OFF
        typedef typename Container::value_type value_type;
        typedef typename Container::reference reference;
        typedef typename Container::const_reference const_reference;
        typedef typename Container::difference_type difference_type;
        typedef typename Container::size_type size_type;

        // We override the iterator types with our constrained wrapper.
        typedef XMLObjectChildrenIterator<Container> iterator;
        typedef XMLObjectChildrenIterator<Container> const_iterator;
        /// @endcond

        /**
         * Constructor to expose a typed collection of children backed by a list of a base type.
         *
         * @param parent    parent object of the collection
         * @param sublist   underlying container to expose
         * @param backing   pointer to backing list for children, if any
         * @param ins_fence a marker designating where new children of this type should be added
         */
        XMLObjectChildrenList(
            XMLObject* parent,
            Container& sublist,
            typename std::list<_Ty*>* backing,
            typename std::list<_Ty*>::iterator ins_fence
            ) : m_container(sublist), m_list(backing), m_fence(ins_fence), m_parent(parent) {
        }

        /// @cond OFF

        size_type size() const {
            // return length of sequence
            return m_container.size();
        }

        bool empty() const {
            // test if sequence is empty
            return m_container.empty();
        }

        iterator begin() {
            // return iterator for beginning of mutable sequence
            return m_container.begin();
        }

        iterator end() {
            // return iterator for end of mutable sequence
            return m_container.end();
        }

        const_iterator begin() const {
            // return iterator for beginning of const sequence
            return m_container.begin();
        }

        const_iterator end() const {
            // return iterator for end of const sequence
            return m_container.end();
        }

        const_reference at(size_type _Pos) const {
            // subscript nonmutable sequence with checking
            return m_container.at(_Pos);
        }

        const_reference operator[](size_type _Pos) const {
            // subscript nonmutable sequence
            return m_container[_Pos];
        }

        const_reference front() const {
            // return first element of nonmutable sequence
            return m_container.front();
        }

        const_reference back() const {
            // return last element of nonmutable sequence
            return m_container.back();
        }

        void push_back(const_reference _Val) {
            setParent(_Val);
            if (m_list)
                m_list->insert(m_fence,_Val);
            m_container.push_back(_Val);
        }

        iterator erase(iterator _Where) {
            removeParent(*_Where);
            if (m_list)
                removeChild(*_Where);
            else
                delete *_Where.m_iter;
            return m_container.erase(_Where.m_iter);
        }

        iterator erase(iterator _First, iterator _Last) {
            for (iterator i=_First; i!=_Last; i++) {
                removeParent(*i);
                if (m_list)
                    removeChild(*i);
                else
                    delete *i.m_iter;
            }
            return m_container.erase(_First.m_iter,_Last.m_iter);
        }

        void clear() {
            erase(begin(),end());
        }

    private:
        void setParent(const_reference _Val) {
            if (_Val->getParent())
                throw XMLObjectException("Child object already has a parent.");
            _Val->setParent(m_parent);
            _Val->releaseParentDOM(true);
        }

        void removeParent(const_reference _Val) {
            if (_Val->getParent()!=m_parent)
                throw XMLObjectException("Child object not owned by this parent.");
            _Val->setParent(NULL);
            m_parent->releaseParentDOM(true);
        }

        void removeChild(const_reference _Val) {
            for (typename std::list<_Ty*>::iterator i=m_list->begin(); i!=m_list->end(); i++) {
                if ((*i)==_Val) {
                    m_list->erase(i);
                    delete _Val;
                    return;
                }
            }
        }
        /// @endcond
    };

    /**
     * STL-compatible container that mediates access to underlying lists of typed XML children
     * that come in pairs.
     * 
     * @param _Tx   the subtype container to encapsulate
     * @param _Ty   the base type in the underlying list (defaults to XMLObject)
     */
    template <class Container, class _Ty>
    class XMLObjectPairList
    {
        Container& m_container;
        typename std::list<_Ty*>* m_list;
        typename std::list<_Ty*>::iterator m_fence;
        XMLObject* m_parent;

    public:
        /// @cond OFF
        typedef typename Container::value_type value_type;
        typedef typename Container::reference reference;
        typedef typename Container::const_reference const_reference;
        typedef typename Container::difference_type difference_type;
        typedef typename Container::size_type size_type;

        // We override the iterator types with our constrained wrapper.
        typedef XMLObjectChildrenIterator<Container> iterator;
        typedef XMLObjectChildrenIterator<Container> const_iterator;
        /// @endcond

        /**
         * Constructor to expose a typed collection of pairs backed by a list of a base type.
         *
         * @param parent    parent object of the collection
         * @param sublist   underlying container to expose
         * @param backing   pointer to backing list for children, if any
         * @param ins_fence a marker designating where new children of this type should be added
         */
        XMLObjectPairList(
            XMLObject* parent,
            Container& sublist,
            typename std::list<_Ty*>* backing,
            typename std::list<_Ty*>::iterator ins_fence
            ) : m_container(sublist), m_list(backing), m_fence(ins_fence), m_parent(parent) {
        }

        /// @cond OFF

        size_type size() const {
            // return length of sequence
            return m_container.size();
        }

        bool empty() const {
            // test if sequence is empty
            return m_container.empty();
        }

        iterator begin() {
            // return iterator for beginning of mutable sequence
            return m_container.begin();
        }

        iterator end() {
            // return iterator for end of mutable sequence
            return m_container.end();
        }

        const_iterator begin() const {
            // return iterator for beginning of const sequence
            return m_container.begin();
        }

        const_iterator end() const {
            // return iterator for end of const sequence
            return m_container.end();
        }

        const_reference at(size_type _Pos) const {
            // subscript nonmutable sequence with checking
            return m_container.at(_Pos);
        }

        const_reference operator[](size_type _Pos) const {
            // subscript nonmutable sequence
            return m_container[_Pos];
        }

        const_reference front() const {
            // return first element of nonmutable sequence
            return m_container.front();
        }

        const_reference back() const {
            // return last element of nonmutable sequence
            return m_container.back();
        }

        void push_back(const_reference _Val) {
            setParent(_Val);
            if (m_list) {
                m_list->insert(m_fence,_Val.first);
                m_list->insert(m_fence,_Val.second);
            }
            m_container.push_back(_Val);
        }

        iterator erase(iterator _Where) {
            removeParent(*_Where);
            if (m_list)
                removeChild(*_Where);
            else {
                delete _Where.m_iter->first;
                delete _Where.m_iter->second;
            }
            return m_container.erase(_Where.m_iter);
        }

        iterator erase(iterator _First, iterator _Last) {
            for (iterator i=_First; i!=_Last; i++) {
                removeParent(*i);
                if (m_list)
                    removeChild(*i);
                else {
                    delete i.m_iter->first;
                    delete i.m_iter->second;
                }
            }
            return m_container.erase(_First,_Last);
        }

        void clear() {
            erase(begin(),end());
        }

    private:
        void setParent(const_reference _Val) {
            if (_Val.first->getParent() || (_Val.second && _Val.second->getParent()))
                throw XMLObjectException("One of the child objects already has a parent.");
            _Val.first->setParent(m_parent);
            if (_Val.second)
                _Val.second->setParent(m_parent);
            _Val.first->releaseParentDOM(true);
        }

        void removeParent(const_reference _Val) {
            if (_Val.first->getParent()!=m_parent || (_Val.second && _Val.second->getParent()!=m_parent))
                throw XMLObjectException("One of the child objects not owned by this parent.");
            _Val.first->setParent(NULL);
            if (_Val.second)
                _Val.second->setParent(NULL);
            m_parent->releaseParentDOM(true);
        }

        void removeChild(const_reference _Val) {
            for (typename std::list<_Ty*>::iterator i=m_list->begin(); i!=m_list->end(); i++) {
                if ((*i)==_Val.first) {
                    typename std::list<_Ty*>::iterator j=i++;
                    m_list->erase(j);
                    m_list->erase(i);
                    delete _Val.first;
                    delete _Val.second;
                    return;
                }
                i++;
            }
        }
        /// @endcond
    };

};

#endif /* __xmltooling_list_h__ */
