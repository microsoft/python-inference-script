/*
Copyright(c) 2013 - 2014, Naoki Yoshinaga
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met :

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the
distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

http://www.tkl.iis.u-tokyo.ac.jp/~ynaga/cedar/
*/
#pragma once
#pragma warning(push)
#pragma warning(disable:4996)

#include <vector>
#include <ostream>
#include <istream>
#include <cstring>
#include <cassert>   // assert
#include <climits>

#include "pyis/share/exception.h"

namespace Cedar
{
    // typedefs
using npos_t = uint64_t;
using uchar_t = uint8_t;
static const npos_t TAIL_OFFSET_MASK = static_cast<npos_t>(0xffffffff);
static const npos_t NODE_INDEX_MASK = static_cast<npos_t>(0xffffffff) << 32;
static const int MAX_ALLOC_SIZE = 1 << 16; // must be divisible by 256

// dynamic double array
template <typename value_t, const bool ORDERED = true, const int MAX_TRIAL = 1, const size_t NUM_TRACKING_NODES = 0> class DoubleArray
{
public:
    enum error_code
    {
        CEDAR_NO_VALUE = INT_MIN, 
        CEDAR_NO_PATH = (INT_MIN+1)
    };
    using result_t = value_t;

    struct result_pair_t
    {
        value_t value;
        size_t length; // prefix length
    };

    // for predict to return result
    struct result_triple_t
    {
        value_t value;
        size_t length; // suffix length
        npos_t id; // node id of value
    };

    struct node_t
    {
        union {
            int base;
            value_t value;
        }; // negative means prev empty index
        int check; // negative means next empty index
        node_t(const int base_ = 0, const int check_ = 0)
            : base(base_)
            , check(check_)
        {
        }
    };

    struct ninfo_t // x1.5 update speed; +.25 % memory (8n -> 10n)
    {
        uchar_t sibling; // right sibling (= 0 if not exist)
        uchar_t child; // first child
        ninfo_t()
            : sibling(0)
            , child(0)
        {
        }
    };

    // a block w/ 256 elements
    struct block_t
    {
        int prev; // prev block; 3 bytes
        int next; // next block; 3 bytes
        short num; // # empty elements; 0 - 256
        short reject; // minimum # branching failed to locate; soft limit
        int trial; // # trial
        int ehead; // first empty item
        block_t()
            : prev(0)
            , next(0)
            , num(256)
            , reject(257)
            , trial(0)
            , ehead(0)
        {
        }
    };

    DoubleArray()
        : tracking_node()
        , m_array(0)
        , m_tail(0)
        , m_tail0(0)
        , m_ninfo(0)
        , m_block(0)
        , m_bheadF(0)
        , m_bheadC(0)
        , m_bheadO(0)
        , m_capacity(0)
        , m_size(0)
        , m_quota(0)
        , m_quota0(0)
        , m_noDelete(false)
        , m_reject()
    {
        static_assert((MAX_ALLOC_SIZE % 256) == 0, "MAX_ALLOC_SIZE must be divisible by 256");
        if (sizeof(value_t) > sizeof(int))
        {
            PYIS_THROW("value type is not supported.");
        }
        Initialize();
    }

    ~DoubleArray()
    {
        Clear(false);
    }

    size_t Capacity() const
    {
        return static_cast<size_t>(m_capacity);
    }

    size_t Size() const
    {
        return static_cast<size_t>(m_size);
    }

    size_t Length() const
    {
        return static_cast<size_t>(*m_length);
    }

    size_t TotalSize() const
    {
        return sizeof(node_t) * m_size;
    }

    size_t UnitSize() const
    {
        return sizeof(node_t);
    }

    size_t NonzeroSize() const
    {
        size_t i = 0;
        for (int to = 0; to < m_size; ++to)
            if (m_array[to].check >= 0)
                ++i;
        return i;
    }

    size_t NonzeroLength() const
    {
        size_t i(0), j(0);
        for (int to = 0; to < m_size; ++to)
        {
            const node_t& n = m_array[to];
            if (n.check >= 0 && m_array[n.check].base != to && n.base < 0)
            {
                ++j;
                for (const char* p = &m_tail[-n.base]; *p; ++p)
                    ++i;
            }
        }
        return i + j * (1 + sizeof(value_t));
    }

    size_t NumKeys() const
    {
        size_t i = 0;
        for (int to = 0; to < m_size; ++to)
        {
            const node_t& n = m_array[to];
            if (n.check >= 0 && (m_array[n.check].base == to || n.base < 0))
                ++i;
        }
        return i;
    }

    // interface
    template <typename T> T ExactMatchSearch(const char* key) const
    {
        return ExactMatchSearch<T>(key, std::strlen(key));
    }

    template <typename T> T ExactMatchSearch(const char* key, size_t len, npos_t from = 0) const
    {
        union {
            int i;
            value_t x;
        } b;
        size_t pos = 0;
        b.i = Find(key, from, pos, len);
        if (b.i == CEDAR_NO_PATH)
            b.i = CEDAR_NO_VALUE;
        T result;
        SetResult(&result, b.x, len, from);
        return result;
    }

    template <typename T> size_t CommonPrefixSearch(const char* key, T* result, size_t result_len) const
    {
        return commonPrefixSearch(key, result, result_len, std::strlen(key));
    }

    template <typename T> size_t CommonPrefixSearch(const char* key, T* result, size_t result_len, size_t len, npos_t from = 0) const
    {
        size_t num = 0;
        for (size_t pos = 0; pos < len;)
        {
            union {
                int i;
                value_t x;
            } b;
            b.i = Find(key, from, pos, pos + 1);
            if (b.i == CEDAR_NO_VALUE)
                continue;
            if (b.i == CEDAR_NO_PATH)
                return num;
            if (num < result_len)
                SetResult(&result[num], b.x, pos, from);
            ++num;
        }
        return num;
    }

    // predict key from double array
    template <typename T> size_t CommonPrefixPredict(const char* key, T* result, size_t result_len)
    {
        return CommonPrefixPredict(key, result, result_len, std::strlen(key));
    }

    template <typename T> size_t CommonPrefixPredict(const char* key, T* result, size_t result_len, size_t len, npos_t from = 0)
    {
        size_t num(0), pos(0), p(0);
        if (Find(key, from, pos, len) == CEDAR_NO_PATH)
            return 0;
        union {
            int i;
            value_t x;
        } b;
        const npos_t root = from;
        for (b.i = Begin(from, p); b.i != CEDAR_NO_PATH; b.i = Next(from, p, root))
        {
            if (num < result_len)
                SetResult(&result[num], b.x, p, from);
            ++num;
        }
        return num;
    }

    void Suffix(char* key, size_t len, npos_t to) const
    {
        key[len] = '\0';
        if (const int offset = static_cast<int>(to >> 32))
        {
            to &= TAIL_OFFSET_MASK;
            size_t len_tail = std::strlen(&m_tail[-m_array[to].base]);
            if (len > len_tail)
                len -= len_tail;
            else
                len_tail = len, len = 0;
            std::memcpy(&key[len], &m_tail[static_cast<size_t>(offset) - len_tail], len_tail);
        }
        while (len--)
        {
            const int from = m_array[to].check;
            key[len] = static_cast<char>(m_array[from].base ^ static_cast<int>(to));
            to = static_cast<npos_t>(from);
        }
    }

    value_t Traverse(const char* key, npos_t& from, size_t& pos) const
    {
        return Traverse(key, from, pos, std::strlen(key));
    }

    value_t Traverse(const char* key, npos_t& from, size_t& pos, size_t len) const
    {
        union {
            int i;
            value_t x;
        } b;
        b.i = Find(key, from, pos, len);
        return b.x;
    }

    struct empty_callback
    {
        void operator()(const int /*unused*/, const int /*unused*/)
        {
        }
    }; // dummy empty function

    value_t& Update(const char* key)
    {
        return Update(key, std::strlen(key));
    }

    value_t& Update(const char* key, size_t len, value_t val = value_t(0))
    {
        npos_t from(0);
        size_t pos(0);
        return Update(key, from, pos, len, val);
    }

    value_t& Update(const char* key, npos_t& from, size_t& pos, size_t len, value_t val = value_t(0))
    {
        empty_callback cf;
        return Update(key, from, pos, len, val, cf);
    }

    template <typename T> value_t& Update(const char* key, npos_t& from, size_t& pos, size_t len, value_t val, T& cf)
    {
        if (!len && !from)
        {
            PYIS_THROW("failed to insert zero-length key");
        }
        npos_t offset = from >> 32;
        if (!offset) // node on trie
        {
            for (const uchar_t* const key_ = reinterpret_cast<const uchar_t*>(key); m_array[from].base >= 0; ++pos)
            {
                if (pos == len)
                {
                    const int to = Follow(from, 0, cf);
                    return m_array[to].value += val;
                }
                from = static_cast<size_t>(Follow(from, key_[pos], cf));
            }
            offset = static_cast<npos_t>(-m_array[from].base);
        }
        if (offset >= sizeof(int)) // go to _tail
        {
            const size_t pos_orig = pos;
            char* const tail = &m_tail[offset] - pos;
            while (pos < len && key[pos] == tail[pos])
                ++pos;
            //
            if (pos == len && tail[pos] == '\0') // found exact key
            {
                if (const npos_t moved = pos - pos_orig) // search end on tail
                {
                    from &= TAIL_OFFSET_MASK;
                    from |= (offset + moved) << 32;
                }
                return *reinterpret_cast<value_t*>(&tail[len + 1]) += val;
            }
            // otherwise, insert the common prefix in tail if any
            if (from >> 32)
            {
                from &= TAIL_OFFSET_MASK; // reset to update tail offset
                for (npos_t offset_ = static_cast<npos_t>(-m_array[from].base); offset_ < offset;)
                {
                    from = static_cast<size_t>(Follow(from, static_cast<uchar_t>(m_tail[offset_]), cf));
                    ++offset_;
                    // this shows intricacy in debugging updatable double array trie
                    if (NUM_TRACKING_NODES) // keep the traversed node (on tail) updated
                        for (size_t j = 0; tracking_node[j] != 0; ++j)
                            if (tracking_node[j] >> 32 == offset_)
                                tracking_node[j] = static_cast<npos_t>(from);
                }
            }
            for (size_t pos_ = pos_orig; pos_ < pos; ++pos_)
                from = static_cast<size_t>(Follow(from, static_cast<uchar_t>(key[pos_]), cf));
            npos_t moved = pos - pos_orig;
            if (tail[pos]) // remember to move offset to existing tail
            {
                const int to_ = Follow(from, static_cast<uchar_t>(tail[pos]), cf);
                m_array[to_].base = -static_cast<int>(offset + ++moved);
                moved -= 1 + sizeof(value_t); // keep record
            }
            moved += offset;
            for (npos_t i = offset; i <= moved; i += 1 + sizeof(value_t))
            {
                if (m_quota0 == ++*m_length0)
                {
                    m_quota0 += *m_length0 >= MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : *m_length0;
                    ReallocArray(m_tail0, m_quota0, *m_length0);
                }
                m_tail0[*m_length0] = static_cast<int>(i);
            }
            if (pos == len || tail[pos] == '\0')
            {
                const int to = Follow(from, 0, cf);
                // set value on tail
                if (pos == len)
                    return m_array[to].value += val;
                m_array[to].value += *reinterpret_cast<value_t*>(&tail[pos + 1]);
            }
            from = static_cast<size_t>(Follow(from, static_cast<uchar_t>(key[pos]), cf));
            ++pos;
        }
        const int needed = static_cast<int>(len - pos + 1 + sizeof(value_t));
        // reuse
        if (pos == len && *m_length0)
        {
            const int offset0 = m_tail0[*m_length0];
            m_tail[offset0] = '\0';
            m_array[from].base = -offset0;
            --*m_length0;
            return *reinterpret_cast<value_t*>(&m_tail[offset0 + 1]) = val;
        }
        if (m_quota < *m_length + needed)
        {
            m_quota += needed > *m_length || needed > MAX_ALLOC_SIZE ? needed : (*m_length >= MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : *m_length);
            ReallocArray(m_tail, m_quota, *m_length);
        }
        m_array[from].base = -*m_length;
        const size_t pos_orig = pos;
        char* const tail = &m_tail[*m_length] - pos;
        if (pos < len)
        {
            do
                tail[pos] = key[pos];
            while (++pos < len);
            from |= (static_cast<npos_t>(*m_length) + (len - pos_orig)) << 32;
        }
        *m_length += needed;
        return *reinterpret_cast<value_t*>(&tail[len + 1]) += val;
    }

    // easy-going erase () without compression
    int Erase(const char* key)
    {
        return Erase(key, std::strlen(key));
    }

    int Erase(const char* key, size_t len, npos_t from = 0)
    {
        size_t pos = 0;
        const int i = Find(key, from, pos, len);
        if (i == CEDAR_NO_PATH || i == CEDAR_NO_VALUE)
            return -1;
        if (from >> 32)
            from &= TAIL_OFFSET_MASK; // leave tail as is
        bool flag = m_array[from].base < 0; // have sibling
        int e = flag ? static_cast<int>(from) : m_array[from].base ^ 0;
        from = m_array[e].check;
        do
        {
            const node_t& n = m_array[from];
            flag = m_ninfo[n.base ^ m_ninfo[from].child].sibling;
            if (flag)
                PopSibling(from, n.base, static_cast<uchar_t>(n.base ^ e));
            PushEnode(e);
            e = static_cast<int>(from);
            from = static_cast<size_t>(m_array[from].check);
        } while (!flag);
        return 0;
    }

    int Build(size_t num, const char** key, const size_t* len = nullptr, const value_t* val = 0)
    {
        for (size_t i = 0; i < num; ++i)
            Update(key[i], len ? len[i] : std::strlen(key[i]), val ? val[i] : value_t(i));
        return 0;
    }

    template <typename T> void Dump(T* result, const size_t result_len)
    {
        union {
            int i;
            value_t x;
        } b;
        size_t num(0), p(0);
        npos_t from = 0;
        for (b.i = Begin(from, p); b.i != CEDAR_NO_PATH; b.i = Next(from, p))
        {
            if (num < result_len)
            {
                SetResult(&result[num++], b.x, p, from);
            }
            else
            {
                PYIS_THROW("dump() needs array of length = num_keys()");
            }
        }
    }

    void ShrinkTail()
    {
        union {
            char* tail;
            int* length;
        } t;
        const size_t length_ = static_cast<size_t>(*m_length) - static_cast<size_t>(*m_length0) * (1 + sizeof(value_t));
        t.tail = static_cast<char*>(std::malloc(length_));
        if (!t.tail)
        {
            PYIS_THROW("memory allocation failed");
        }
        *t.length = static_cast<int>(sizeof(int));
        for (int to = 0; to < m_size; ++to)
        {
            node_t& n = m_array[to];
            if (n.check >= 0 && m_array[n.check].base != to && n.base < 0)
            {
                char *const tail(&t.tail[*t.length]), *const tail_(&m_tail[-n.base]);
                n.base = -*t.length;
                int i = 0;
                do
                    tail[i] = tail_[i];
                while (tail[i++]);
                *reinterpret_cast<value_t*>(&tail[i]) = *reinterpret_cast<const value_t*>(&tail_[i]);
                *t.length += i + static_cast<int>(sizeof(value_t));
            }
        }
        std::free(m_tail);
        m_tail = t.tail;
        ReallocArray(m_tail, *m_length, *m_length);
        m_quota = *m_length;
        ReallocArray(m_tail0, 1);
        m_quota0 = 1;
    }

    void Save(std::ostream& os, bool shrink)
    {
        if (shrink)
            ShrinkTail();
        Save(os);
    }

    void Save(std::ostream& os) const
    {
        // _test ();
        os.write((char*)m_length, sizeof(*m_length));
        os.write(m_tail, *m_length);
        
        os.write((char*)&m_size, sizeof(m_size));
        os.write((char*)m_array, sizeof(node_t) * m_size);
    }

    void Open(std::istream& is)
    {
        // set array
        Clear(false);

        int tail_len = 0;
        is.read((char*)&tail_len, sizeof(tail_len));
        m_tail = static_cast<char*>(std::malloc(tail_len));
        if (!m_tail) {
            PYIS_THROW("memory allocation failed");
        }
        is.read(m_tail, tail_len);

        is.read((char*)&m_size, sizeof(m_size));
        m_array = static_cast<node_t*>(std::malloc(sizeof(node_t) * m_size));
        if (!m_array) {
            PYIS_THROW("memory allocation failed");
        }
        is.read((char*)m_array, sizeof(node_t) * m_size);
        
        m_tail0 = static_cast<int*>(std::malloc(sizeof(int)));
        if (!m_tail0)
        {
            PYIS_THROW("memory allocation failed");
        }

        *m_length0 = 0;
    }

    void Restore() // restore information to update
    {
        if (!m_block)
            RestoreBlock();
        if (!m_ninfo)
            RestoreNinfo();
        m_capacity = m_size;
        m_quota = *m_length;
        m_quota0 = 1;
    }

    void SetArray(void* p, size_t size_ = 0) // ad-hoc
    {
        Clear(false);
        if (size_)
            size_ = size_ * UnitSize() - static_cast<size_t>(*static_cast<int*>(p));
        m_tail = static_cast<char*>(p);
        m_array = reinterpret_cast<node_t*>(m_tail + *m_length);
        m_size = static_cast<int>(size_ / UnitSize() + (size_ % UnitSize() ? 1 : 0));
        m_noDelete = true;
    }

    const void* Array() const
    {
        return m_array;
    }

    void Clear(const bool reuse = true)
    {
        if (m_noDelete)
            m_array = 0, m_tail = 0;
        if (m_array)
            std::free(m_array);
        m_array = 0;
        if (m_tail)
            std::free(m_tail);
        m_tail = 0;
        if (m_tail0)
            std::free(m_tail0);
        m_tail0 = 0;
        if (m_ninfo)
            std::free(m_ninfo);
        m_ninfo = 0;
        if (m_block)
            std::free(m_block);
        m_block = 0;
        m_bheadF = m_bheadC = m_bheadO = m_capacity = m_size = m_quota = m_quota0 = 0;
        if (reuse)
            Initialize();
        m_noDelete = false;
    }

    // return the first child for a tree rooted by a given node
    int Begin(npos_t& from, size_t& len)
    {
        if (!m_ninfo)
            RestoreNinfo();
        int base = from >> 32 ? -static_cast<int>(from >> 32) : m_array[from].base;
        if (base >= 0) // on trie
        {
            uchar_t c = m_ninfo[from].child;
            if (!from && !(c = m_ninfo[base ^ c].sibling)) // bug fix
                return CEDAR_NO_PATH; // no entry
            for (; c && base >= 0; ++len)
            {
                from = static_cast<size_t>(base) ^ c;
                base = m_array[from].base;
                c = m_ninfo[from].child;
            }
            if (base >= 0)
                return m_array[base ^ c].base;
        }
        const size_t len_ = std::strlen(&m_tail[-base]);
        from &= TAIL_OFFSET_MASK;
        from |= static_cast<npos_t>(static_cast<size_t>(-base) + len_) << 32;
        len += len_;
        return *reinterpret_cast<int*>(&m_tail[-base] + len_ + 1);
    }

    // return the next child if any
    int Next(npos_t& from, size_t& len, const npos_t root = 0)
    {
        uchar_t c = 0;
        if (const int offset = static_cast<int>(from >> 32)) // on tail
        {
            if (root >> 32)
                return CEDAR_NO_PATH;
            from &= TAIL_OFFSET_MASK;
            len -= static_cast<size_t>(offset - (-m_array[from].base));
        }
        else
            c = m_ninfo[m_array[from].base ^ 0].sibling;
        for (; !c && from != root; --len)
        {
            c = m_ninfo[from].sibling;
            from = static_cast<size_t>(m_array[from].check);
        }
        if (!c)
            return CEDAR_NO_PATH;
        return Begin(from = static_cast<size_t>(m_array[from].base) ^ c, ++len);
    }
    npos_t tracking_node[NUM_TRACKING_NODES + 1];

private:
    // currently disabled; implement these if you need
    DoubleArray(const DoubleArray&);
    DoubleArray& operator=(const DoubleArray&);
    node_t* m_array;
    union {
        char* m_tail;
        int* m_length;
    };
    union {
        int* m_tail0;
        int* m_length0;
    };
    ninfo_t* m_ninfo;
    block_t* m_block;
    int m_bheadF; // first block of Full;   0
    int m_bheadC; // first block of Closed; 0 if no Closed
    int m_bheadO; // first block of Open;   0 if no Open
    int m_capacity;
    int m_size;
    int m_quota;
    int m_quota0;
    int m_noDelete;
    short m_reject[257];

    template <typename T> static void ReallocArray(T*& p, const int size_n, const int size_p = 0)
    {
        void* tmp = std::realloc(p, sizeof(T) * static_cast<size_t>(size_n));
        if (!tmp)
        {
            PYIS_THROW("memory reallocation failed");
        }
        p = static_cast<T*>(tmp);
        static const T T0 = T();
        for (T *q(p + size_p), *const r(p + size_n); q != r; ++q)
            *q = T0;
    }

    void Initialize() // initilize the first special block
    {
        ReallocArray(m_array, 256, 256);
        ReallocArray(m_tail, sizeof(int));
        ReallocArray(m_tail0, 1);
        ReallocArray(m_ninfo, 256);
        ReallocArray(m_block, 1);
        m_array[0] = node_t(0, -1);
        for (int i = 1; i < 256; ++i)
            m_array[i] = node_t(i == 1 ? -255 : -(i - 1), i == 255 ? -1 : -(i + 1));
        m_capacity = m_size = 256;
        m_block[0].ehead = 1; // bug fix for erase
        m_quota = *m_length = static_cast<int>(sizeof(int));
        m_quota0 = 1;
        for (size_t i = 0; i <= NUM_TRACKING_NODES; ++i)
            tracking_node[i] = 0;
        for (short i = 0; i <= 256; ++i)
            m_reject[i] = i + 1;
    }

    // follow/create edge
    template <typename T> int Follow(npos_t& from, const uchar_t& label, T& cf)
    {
        int to = 0;
        const int base = m_array[from].base;
        if (base < 0 || m_array[to = base ^ label].check < 0)
        {
            to = PopEnode(base, label, static_cast<int>(from));
            PushSibling(from, to ^ label, label, base >= 0);
        }
        else if (m_array[to].check != static_cast<int>(from))
            to = Resolve(from, base, label, cf);
        return to;
    }

    // find key from double array
    int Find(const char* key, npos_t& from, size_t& pos, const size_t len) const
    {
        npos_t offset = from >> 32;
        if (!offset) // node on trie
        {
            for (const uchar_t* const key_ = reinterpret_cast<const uchar_t*>(key); m_array[from].base >= 0;)
            {
                if (pos == len)
                {
                    const node_t& n = m_array[m_array[from].base ^ 0];
                    if (n.check != static_cast<int>(from))
                        return CEDAR_NO_VALUE;
                    return n.base;
                }
                size_t to = static_cast<size_t>(m_array[from].base);
                to ^= key_[pos];
                if (m_array[to].check != static_cast<int>(from))
                    return CEDAR_NO_PATH;
                ++pos;
                from = to;
            }
            offset = static_cast<npos_t>(-m_array[from].base);
        }
        // switch to _tail to match suffix
        const size_t pos_orig = pos; // start position in reading _tail
        const char* const tail = &m_tail[offset] - pos;
        if (pos < len)
        {
            do
                if (key[pos] != tail[pos])
                    break;
            while (++pos < len);
            if (const npos_t moved = pos - pos_orig)
            {
                from &= TAIL_OFFSET_MASK;
                from |= (offset + moved) << 32;
            }
            // input > tail, input != tail
            if (pos < len)
                return CEDAR_NO_PATH;
        }
        // input < tail
        if (tail[pos])
            return CEDAR_NO_VALUE;
        return *reinterpret_cast<const int*>(&tail[len + 1]);
    }

    void RestoreNinfo()
    {
        ReallocArray(m_ninfo, m_size);
        for (int to = 0; to < m_size; ++to)
        {
            const int from = m_array[to].check;
            // skip empty node
            if (from < 0)
                continue;
            const int base = m_array[from].base;
            // skip leaf
            if (const uchar_t label = static_cast<uchar_t>(base ^ to))
                PushSibling(static_cast<size_t>(from), base, label, !from || m_ninfo[from].child || m_array[base ^ 0].check == from);
        }
    }

    void RestoreBlock()
    {
        ReallocArray(m_block, m_size >> 8);
        m_bheadF = m_bheadC = m_bheadO = 0;
        // register blocks to full
        for (int bi(0), e(0); e < m_size; ++bi)
        {
            block_t& b = m_block[bi];
            b.num = 0;
            for (; e < (bi << 8) + 256; ++e)
                if (m_array[e].check < 0 && ++b.num == 1)
                    b.ehead = e;
            int& head_out = b.num == 1 ? m_bheadC : (b.num == 0 ? m_bheadF : m_bheadO);
            PushBlock(bi, head_out, !head_out && b.num);
        }
    }

    void SetResult(result_t* x, value_t r, size_t /*unused*/ = 0, npos_t /*unused*/ = 0) const
    {
        *x = r;
    }

    void SetResult(result_pair_t* x, value_t r, size_t l, npos_t /*unused*/ = 0) const
    {
        x->value = r;
        x->length = l;
    }

    void SetResult(result_triple_t* x, value_t r, size_t l, npos_t from) const
    {
        x->value = r;
        x->length = l;
        x->id = from;
    }

    void PopBlock(const int bi, int& head_in, const bool last)
    {
        // last one poped; Closed or Open
        if (last)
        {
            head_in = 0;
        }
        else
        {
            const block_t& b = m_block[bi];
            m_block[b.prev].next = b.next;
            m_block[b.next].prev = b.prev;
            if (bi == head_in)
                head_in = b.next;
        }
    }

    void PushBlock(const int bi, int& head_out, const bool empty)
    {
        block_t& b = m_block[bi];
        if (empty) // the destination is empty
        {
            head_out = b.prev = b.next = bi;
        }
        else // use most recently pushed
        {
            int& tail_out = m_block[head_out].prev;
            b.prev = tail_out;
            b.next = head_out;
            head_out = tail_out = m_block[tail_out].next = bi;
        }
    }

    int AddBlock()
    {
        // allocate memory if needed
        if (m_size == m_capacity)
        {
            m_capacity += m_size >= MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : m_size;
            ReallocArray(m_array, m_capacity, m_capacity);
            ReallocArray(m_ninfo, m_capacity, m_size);
            ReallocArray(m_block, m_capacity >> 8, m_size >> 8);
        }
        m_block[m_size >> 8].ehead = m_size;
        m_array[m_size] = node_t(-(m_size + 255), -(m_size + 1));
        for (int i = m_size + 1; i < m_size + 255; ++i)
            m_array[i] = node_t(-(i - 1), -(i + 1));
        m_array[m_size + 255] = node_t(-(m_size + 254), -m_size);
        // append to block Open
        PushBlock(m_size >> 8, m_bheadO, !m_bheadO);
        m_size += 256;
        return (m_size >> 8) - 1;
    }

    // transfer block from one start w/ head_in to one start w/ head_out
    void TransferBlock(const int bi, int& head_in, int& head_out)
    {
        PopBlock(bi, head_in, bi == m_block[bi].next);
        PushBlock(bi, head_out, !head_out && m_block[bi].num);
    }

    // pop empty node from block; never transfer the special block (bi = 0)
    int PopEnode(const int base, const uchar_t label, const int from)
    {
        const int e = base < 0 ? FindPlace() : base ^ label;
        const int bi = e >> 8;
        node_t& n = m_array[e];
        block_t& b = m_block[bi];
        if (--b.num == 0)
        {
            // Closed to Full
            if (bi)
                TransferBlock(bi, m_bheadC, m_bheadF);
        }
        else // release empty node from empty ring
        {
            m_array[-n.base].check = n.check;
            m_array[-n.check].base = n.base;
            // set ehead
            if (e == b.ehead)
                b.ehead = -n.check;
            // Open to Closed
            if (bi && b.num == 1 && b.trial != MAX_TRIAL)
                TransferBlock(bi, m_bheadO, m_bheadC);
        }
        // initialize the released node
        if (label)
            n.base = -1;
        else
            n.value = value_t(0);
        n.check = from;
        if (base < 0)
            m_array[from].base = e ^ label;
        return e;
    }

    // push empty node into empty ring
    void PushEnode(const int e)
    {
        const int bi = e >> 8;
        block_t& b = m_block[bi];
        // Full to Closed
        if (++b.num == 1)
        {
            b.ehead = e;
            m_array[e] = node_t(-e, -e);
            // Full to Closed
            if (bi)
                TransferBlock(bi, m_bheadF, m_bheadC);
        }
        else
        {
            const int prev = b.ehead;
            const int next = -m_array[prev].check;
            m_array[e] = node_t(-prev, -next);
            m_array[prev].check = m_array[next].base = -e;
            // Closed to Open
            if (b.num == 2 || b.trial == MAX_TRIAL)
            {
                if (bi)
                    TransferBlock(bi, m_bheadC, m_bheadO);
            }
            b.trial = 0;
        }
        if (b.reject < m_reject[b.num])
            b.reject = m_reject[b.num];
        // reset ninfo; no child, no sibling
        m_ninfo[e] = ninfo_t();
    }

    // push label to from's child
    void PushSibling(const npos_t from, const int base, const uchar_t label, const bool flag = true)
    {
        uchar_t* c = &m_ninfo[from].child;
        if (flag && (ORDERED ? label > *c : !*c))
            do
                c = &m_ninfo[base ^ *c].sibling;
            while (ORDERED && *c && *c < label);
            m_ninfo[base ^ label].sibling = *c, *c = label;
        }

        // pop label from from's child
        void PopSibling(const npos_t from, const int base, const uchar_t label)
        {
            uchar_t* c = &m_ninfo[from].child;
            while (*c != label) c = &m_ninfo[base ^ *c].sibling;
            *c = m_ninfo[base ^ label].sibling;
        }

        // check whether to replace branching w/ the newly added node
        bool Consult(const int base_n, const int base_p, uchar_t c_n,
            uchar_t c_p) const
        {
            do
                c_n = m_ninfo[base_n ^ c_n].sibling, c_p = m_ninfo[base_p ^ c_p].sibling;
            while (c_n && c_p);
            return c_p;
        }

        // enumerate (equal to or more than one) child nodes
        uchar_t* SetChild(uchar_t* p, const int base, uchar_t c, const int label = -1)
        {
            --p;
            if (!c)
            {
                *++p = c;
                c = m_ninfo[base ^ c].sibling;
            }  // 0: terminal
            if (ORDERED)
                while (c && c < label)
                {
                    *++p = c;
                    c = m_ninfo[base ^ c].sibling;
                }
            if (label != -1) *++p = static_cast<uchar_t>(label);
            while (c)
            {
                *++p = c;
                c = m_ninfo[base ^ c].sibling;
            }
            return p;
        }

        // explore new block to settle down
        int FindPlace()
        {
            if (m_bheadC) return m_block[m_bheadC].ehead;
            if (m_bheadO) return m_block[m_bheadO].ehead;
            return AddBlock() << 8;
        }

        int FindPlace(const uchar_t* const first, const uchar_t* const last)
        {
            if (int bi = m_bheadO)
            {
                const int bz = m_block[m_bheadO].prev;
                const short nc = static_cast<short>(last - first + 1);
                // set candidate block
                while (1)
                {
                    block_t& b = m_block[bi];
                    // explore configuration
                    if (b.num >= nc && nc < b.reject)
                        for (int e = b.ehead;;)
                        {
                            const int base = e ^ *first;
                            for (const uchar_t* p = first; m_array[base ^ *++p].check < 0;)
                                if (p == last) return b.ehead = e;  // no conflict
                            if ((e = -m_array[e].check) == b.ehead) break;
                        }
                    b.reject = nc;
                    if (b.reject < m_reject[b.num]) m_reject[b.num] = b.reject;
                    const int bi_ = b.next;
                    if (++b.trial == MAX_TRIAL) TransferBlock(bi, m_bheadO, m_bheadC);
                    if (bi == bz) break;
                    bi = bi_;
                }
            }
            return AddBlock() << 8;
        }

        // resolve conflict on base_n ^ label_n = base_p ^ label_p
        template <typename T>
        int Resolve(npos_t& from_n, const int base_n, const uchar_t label_n, T& cf)
        {
            // examine siblings of conflicted nodes
            const int to_pn = base_n ^ label_n;
            const int from_p = m_array[to_pn].check;
            const int base_p = m_array[from_p].base;
            // whether to replace siblings of newly added
            const bool flag
                = Consult(base_n, base_p, m_ninfo[from_n].child, m_ninfo[from_p].child);
            uchar_t child[256];
            uchar_t* const first = &child[0];
            uchar_t* const last =
                flag ? SetChild(first, base_n, m_ninfo[from_n].child, label_n)
                : SetChild(first, base_p, m_ninfo[from_p].child);
            const int base =
                (first == last ? FindPlace() : FindPlace(first, last)) ^ *first;
            // replace & modify empty list
            const int from = flag ? static_cast<int>(from_n) : from_p;
            const int base_ = flag ? base_n : base_p;
            if (flag && *first == label_n) m_ninfo[from].child = label_n;  // new child
            m_array[from].base = base;                                     // new base
            for (const uchar_t* p = first; p <= last; ++p)                 // to_ => to
            {
                const int to = PopEnode(base, *p, from);
                const int to_ = base_ ^ *p;
                m_ninfo[to].sibling = (p == last ? 0 : *(p + 1));
                // skip newcomer (no child)
                if (flag && to_ == to_pn) continue;
                cf(to_, to);
                node_t& n = m_array[to];
                node_t& n_ = m_array[to_];
                if ((n.base = n_.base) > 0 && *p)    // copy base; bug fix
                {
                    uchar_t c = m_ninfo[to].child = m_ninfo[to_].child;
                    do
                        m_array[n.base ^ c].check = to;  // adjust grand son's check
                    while ((c = m_ninfo[n.base ^ c].sibling));
                }
                if (!flag && to_ == static_cast<int>(from_n))  // parent node moved
                    from_n = static_cast<size_t>(to);          // bug fix
                if (!flag && to_ == to_pn)    // the address is immediately used
                {
                    PushSibling(from_n, to_pn ^ label_n, label_n);
                    m_ninfo[to_].child = 0;  // remember to reset child
                    if (label_n)
                        n_.base = -1;
                    else
                        n_.value = value_t(0);
                    n_.check = static_cast<int>(from_n);
                }
                else
                    PushEnode(to_);
                if (NUM_TRACKING_NODES)  // keep the traversed node updated
                    for (size_t j = 0; tracking_node[j] != 0; ++j)
                    {
                        if (static_cast<int>(tracking_node[j] & TAIL_OFFSET_MASK) == to_)
                        {
                            tracking_node[j] &= NODE_INDEX_MASK;
                            tracking_node[j] |= static_cast<npos_t>(to);
                        }
                    }
            }
            return flag ? base ^ label_n : to_pn;
        }

        // test the validity of double array for debug
        void Test(const npos_t from = 0) const
        {
            const int base = m_array[from].base;
            if (base < 0)    // validate tail offset
            {
                assert(*m_length >= static_cast<int>(-base + 1 + sizeof(value_t)));
                return;
            }
            uchar_t c = m_ninfo[from].child;
            do
            {
                if (from) assert(m_array[base ^ c].check == static_cast<int>(from));
                if (c) Test(static_cast<npos_t>(base ^ c));
            } while ((c = m_ninfo[base ^ c].sibling));
        }
    };

    using npos_t = Cedar::npos_t;
    using trie_t = Cedar::DoubleArray<int>;
     
    // return type for prefix ()
    class TrieResult
    {
    public:
        TrieResult(const TrieResult& r)
            :m_t(r.m_t), m_id(r.m_id), m_len(r.m_len),
            m_val(r.m_val), m_key(r.m_key)
        {
        }

        TrieResult(trie_t* t = nullptr, const npos_t id = 0, const size_t len = 0, const int val = 0)
            : m_t(t)
            , m_id(id)
            , m_len(len)
            , m_val(val)
            , m_key()
        {
        }

        ~TrieResult() = default;

        void Reset(const npos_t id, const size_t len, const int val)
        {
            m_id = id;
            m_len = len;
            m_val = val;
            m_key.clear();
        }

        const char* Key()
        {
            if (m_key.empty())
            {
                m_key.resize(m_len + 1);
                m_t->Suffix(&m_key[0], m_len, m_id);
            }
            return &m_key[0];
        }

        int Value() const
        {
            return m_val;
        }

    protected:
        trie_t* m_t;
        npos_t m_id;
        size_t m_len;
        int m_val;

    private:
        std::vector<char> m_key;
    };

    // return type (iterator) for predict ()
    class TrieIterator : public TrieResult
    {
    private:
        const npos_t m_root;
        TrieResult m_res;

    public:
        TrieIterator(const TrieIterator& r)
            : TrieResult(r), m_root(r.m_root), m_res(r.m_res)
        {
        }

        TrieIterator(trie_t* t,
            const npos_t root = 0,
            const npos_t id = 0,
            const size_t len = 0,
            const int val = 0)
            : TrieResult(t, id, len, val), m_root(root), m_res(t)
        {
        }

        ~TrieIterator() = default;

        const TrieResult* Next()
        {
            if (m_val == trie_t::CEDAR_NO_PATH)
                return nullptr;
            m_res.Reset(m_id, m_len, m_val);
            m_val = m_t->Next(m_id, m_len, m_root);
            return &m_res;
        }
    };

    // interface for script languages; you may want to extend this
    class Trie
    {
    protected:
        trie_t* m_t;
        size_t n_numKeys{0};

    public:
        Trie()
            : m_t(new Cedar::trie_t())
        {
        }

        ~Trie()
        {
            delete m_t;
        }

        // read/write
        void Open(std::istream& is)
        {
            m_t->Open(is);
            n_numKeys = m_t->NumKeys();
        }

        void Save(std::ostream& os)
        {
            // shrink tail and before writing to disk
            m_t->Save(os, true);
        }

        // get statistics
        size_t NumKeys() const
        {
            return n_numKeys;    // O(1)
        }

        // low-level predicates
        int Insert(const char* key, int n = 0)
        {
            npos_t from = 0;
            size_t pos(0), len(std::strlen(key));
            const int n_ = m_t->Traverse(key, from, pos, len);
            bool flag = n_ == trie_t::CEDAR_NO_VALUE || n_ == trie_t::CEDAR_NO_PATH;
            if (flag)
            {
                ++n_numKeys;
            }
            m_t->Update(key, from, pos, len) = n;
            return flag ? 0 : -1;
        }

        int Erase(const char* key)
        {
            if (m_t->Erase(key))
                return -1;

            --n_numKeys;
            return 0;
        }

        int Lookup(const char* key) const
        {
            return m_t->ExactMatchSearch<trie_t::result_t>(key);
        }

        // high-level (trie-specific) predicates
        std::vector<TrieResult> Prefix(const char* key) const
        {
            std::vector<TrieResult> result;
            std::vector<trie_t::result_triple_t> resultTriples;
            const size_t len = std::strlen(key);
            resultTriples.resize(len);
            result.resize(m_t->CommonPrefixSearch(key, &resultTriples[0], len, len), m_t);
            for (size_t i = 0; i != result.size(); ++i)
            {
                result[i].Reset(resultTriples[i].id, resultTriples[i].length, resultTriples[i].value);
            }
            return result;
        }

        TrieResult LongestPrefix(const char* key) const
        {
            TrieResult r(m_t, 0, 0, trie_t::CEDAR_NO_VALUE);  // result for not found
            npos_t from = 0;
            for (size_t pos(0), len(std::strlen(key)); pos < len;)
            {
                const int n = m_t->Traverse(key, from, pos, pos + 1);
                if (n == trie_t::CEDAR_NO_PATH) break;
                if (n != trie_t::CEDAR_NO_VALUE) r.Reset(from, pos, n);
            }
            return r;
        }

        TrieIterator Predict(const char* key) const
        {
            npos_t from = 0;
            size_t pos(0), len(0);
            int n = m_t->Traverse(key, from, pos);
            npos_t root = from;
            if (n != trie_t::CEDAR_NO_PATH) n = m_t->Begin(from, len);
            return TrieIterator(m_t, root, from, len, n);
        }

        void Dump(std::vector<std::tuple<std::string, int>>& result) const {
            result.clear();
            size_t len = 0;
            int ret = 0;
            char* buffer;
            uint64_t addr = 0;
            ret = m_t->Begin(addr, len);
            while (ret >= 0) {
                buffer = new char[len + 1];
                m_t->Suffix(buffer, len, addr);
                int val = m_t->ExactMatchSearch<int>(buffer);
                result.emplace_back(std::make_tuple(std::string(buffer), val));
                ret = m_t->Next(addr, len);
                delete[] buffer;
            }
        }
    };
} // namespace Cedar

#pragma warning(pop)
