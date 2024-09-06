// memory_resource_p1160                                              -*-C++-*-
#ifndef BEMANPROJECT_PMR_TEST_RESOURCE_P1160R3_HPP_INCLUDED
#define BEMANPROJECT_PMR_TEST_RESOURCE_P1160R3_HPP_INCLUDED

#include <memory_resource>

#include <atomic>
#include <mutex>
#include <new>
#include <stdexcept>
#include <string_view>

#include <cstdio>
#include <cassert>

namespace beman::pmr {

struct test_resource_list;

class test_resource : public std::pmr::memory_resource {

    std::string_view           m_name{};
                              
    std::atomic_int            m_no_abort_flag{ false };
    std::atomic_int            m_quiet_flag{ false };
    std::atomic_int            m_verbose_flag{ false };
    std::atomic_llong          m_allocation_limit{ -1 };
                              
    std::atomic_llong          m_allocations{ 0 };
    std::atomic_llong          m_deallocations{ 0 };
    std::atomic_llong          m_mismatches{ 0 };
    std::atomic_llong          m_bounds_errors{ 0 };
    std::atomic_llong          m_bad_deallocate_params{ 0 };
                              
    std::atomic_llong          m_blocks_in_use{ 0 };
    std::atomic_llong          m_max_blocks{ 0 };
    std::atomic_llong          m_total_blocks{ 0 };
                              
    std::atomic_size_t         m_bytes_in_use{ 0 };
    std::atomic_llong          m_max_bytes{ 0 };
    std::atomic_size_t         m_total_bytes{ 0 };
                              
    std::atomic_size_t         m_last_allocated_num_bytes{ 0 };
    std::atomic_size_t         m_last_allocated_alignment{ 0 };
    std::atomic<void *>        m_last_allocated_address{ nullptr };
                              
    std::atomic_size_t         m_last_deallocated_num_bytes{ 0 };
    std::atomic_size_t         m_last_deallocated_alignment{ 0 };
    std::atomic<void *>        m_last_deallocated_address{ nullptr };
                              
    test_resource_list        *m_list{};
                              
    mutable std::mutex         m_lock{};

    std::pmr::memory_resource *m_pmr{};

private:
    [[nodiscard]] void *do_allocate(std::size_t bytes,
                                    std::size_t alignment) override;

    void do_deallocate(void        *p,
                       std::size_t  bytes,
                       std::size_t  alignment) override;

    bool do_is_equal(const memory_resource& that) const noexcept override;

public:
    test_resource(const test_resource&) = delete;
    test_resource& operator=(const test_resource&) = delete;

    test_resource();
    explicit test_resource(std::pmr::memory_resource *pmrp);
    
    explicit test_resource(std::string_view  name);
    explicit test_resource(const char       *name);
    
    explicit test_resource(bool verbose);
    
    test_resource(std::string_view  name, std::pmr::memory_resource *pmrp);
    test_resource(const char       *name, std::pmr::memory_resource *pmrp);
    
    test_resource(bool verbose, std::pmr::memory_resource *pmrp);
    
    test_resource(const char       *name, bool verbose);
    test_resource(std::string_view  name, bool verbose);
    test_resource(std::string_view           name,
                  bool                       verbose,
                  std::pmr::memory_resource *pmrp);
    test_resource(const char                *name,
                  bool                       verbose,
                  std::pmr::memory_resource *pmrp);

    ~test_resource();

    void set_allocation_limit(long long limit) noexcept
    {
        m_allocation_limit.store(limit, std::memory_order_relaxed);
    }

    void set_no_abort(bool is_no_abort) noexcept
    {
        m_no_abort_flag.store(is_no_abort, std::memory_order_relaxed);
    }

    void set_quiet(bool is_quiet) noexcept
    {
        m_quiet_flag.store(is_quiet, std::memory_order_relaxed);
    }

    void set_verbose(bool is_verbose) noexcept
    {
        m_verbose_flag.store(is_verbose, std::memory_order_relaxed);
        m_verbose_flag.store(is_verbose, std::memory_order_relaxed);
    }

    long long allocation_limit() const noexcept
    {
        return m_allocation_limit.load(std::memory_order_relaxed);
    }

    bool is_no_abort() const noexcept
    {
        return m_no_abort_flag.load(std::memory_order_relaxed);
    }

    bool is_quiet() const noexcept
    {
        return m_quiet_flag.load(std::memory_order_relaxed);
    }

    bool is_verbose() const noexcept
    {
        return m_verbose_flag.load(std::memory_order_relaxed);
    }

    std::string_view name() const noexcept
    {
        return m_name;
    }

    std::pmr::memory_resource *upstream_resource() const noexcept
    {
        return m_pmr;
    }

    void *last_allocated_address() const noexcept
    {
        return m_last_allocated_address.load(std::memory_order_relaxed);
    }

    std::size_t last_allocated_num_bytes() const noexcept
    {
        return m_last_allocated_num_bytes.load(std::memory_order_relaxed);
    }

    std::size_t last_allocated_aligment() const noexcept
    {
        return m_last_allocated_alignment.load(std::memory_order_relaxed);
    }

    void *last_deallocated_address() const noexcept
    {
        return m_last_deallocated_address.load(std::memory_order_relaxed);
    }

    std::size_t last_deallocated_num_bytes() const noexcept
    {
        return m_last_deallocated_num_bytes.load(std::memory_order_relaxed);
    }

    std::size_t last_deallocated_aligment() const noexcept
    {
        return m_last_deallocated_alignment.load(std::memory_order_relaxed);
    }

    long long allocations() const noexcept
    {
        return m_allocations.load(std::memory_order_relaxed);
    }

    long long deallocations() const noexcept
    {
        return m_deallocations.load(std::memory_order_relaxed);
    }

    long long blocks_in_use() const noexcept
    {
        return m_blocks_in_use.load(std::memory_order_relaxed);
    }

    long long max_blocks() const noexcept
    {
        return m_max_blocks.load(std::memory_order_relaxed);
    }

    long long total_blocks() const noexcept
    {
        return m_total_blocks.load(std::memory_order_relaxed);
    }

    long long bounds_errors() const noexcept
    {
        return m_bounds_errors.load(std::memory_order_relaxed);
    }

    long long bad_deallocate_params() const noexcept
    {
        return m_bad_deallocate_params.load(std::memory_order_relaxed);
    }

    long long bytes_in_use() const noexcept
    {
        return m_bytes_in_use.load(std::memory_order_relaxed);
    }

    long long max_bytes() const noexcept
    {
        return m_max_bytes.load(std::memory_order_relaxed);
    }

    long long total_bytes() const noexcept
    {
        return m_total_bytes.load(std::memory_order_relaxed);
    }

    long long mismatches() const noexcept
    {
        return m_mismatches.load(std::memory_order_relaxed);
    }

    void print() const noexcept;

    bool has_errors() const noexcept
    {
        return mismatches() != 0 || bounds_errors() != 0 ||
               bad_deallocate_params() != 0;
    }

    bool has_allocations() const noexcept
    {
        return blocks_in_use() > 0 || bytes_in_use() > 0;
    }

    long long status() const noexcept;
};


class test_resource_exception : public std::bad_alloc {

    test_resource *m_originating;
    std::size_t    m_size;
    std::size_t    m_alignment;

  public:
    test_resource_exception(test_resource *originating,
                            std::size_t    size,
                            std::size_t    alignment) noexcept
    : m_originating(originating)
    , m_size(size)
    , m_alignment(alignment)
    {
    }

    const char *what() const noexcept override
    {
        return "beman::pmr::test_resource_exception";
    }

    test_resource *originating_resource() const noexcept
    {
        return m_originating;
    }

    std::size_t size() const noexcept
    {
        return m_size;
    }

    std::size_t alignment() const noexcept
    {
        return m_alignment;
    }
};


class test_resource_monitor {

    long long            m_initial_in_use;
    long long            m_initial_max;
    long long            m_initial_total;
    const test_resource& m_monitored;

  public:
    explicit test_resource_monitor(const test_resource& monitored) noexcept
    : m_monitored(monitored)
    {
        reset();
    }
    explicit test_resource_monitor(test_resource&&) = delete;
      // To avoid binding the const ref arg to a temporary (above).

    void reset() noexcept
    {
        m_initial_in_use = m_monitored.blocks_in_use();
        m_initial_max    = m_monitored.max_blocks();
        m_initial_total  = m_monitored.total_blocks();
    }

    bool is_in_use_down() const noexcept
    {
        return m_monitored.blocks_in_use() < m_initial_in_use;
    }

    bool is_in_use_same() const noexcept
    {
        return m_monitored.blocks_in_use() == m_initial_in_use;
    }

    bool is_in_use_up() const noexcept
    {
        return m_monitored.blocks_in_use() > m_initial_in_use;
    }

    bool is_max_same() const noexcept
    {
        return m_initial_max == m_monitored.max_blocks();
    }

    bool is_max_up() const noexcept
    {
        return m_monitored.max_blocks() != m_initial_max;
    }

    bool is_total_same() const noexcept
    {
        return m_monitored.total_blocks() == m_initial_total;
    }

    bool is_total_up() const noexcept
    {
        return m_monitored.total_blocks() != m_initial_total;
    }

    long long delta_blocks_in_use() const noexcept
    {
        return m_monitored.blocks_in_use() - m_initial_in_use;
    }

    long long delta_max_blocks() const noexcept
    {
        return m_monitored.max_blocks() - m_initial_max;
    }

    long long delta_total_blocks() const noexcept
    {
        return m_monitored.total_blocks() - m_initial_total;
    }
};


template <class CODE_BLOCK>
void exception_test_loop(test_resource& pmrp, CODE_BLOCK codeBlock)
{
    for (long long exceptionCounter = 0; true; ++exceptionCounter) {
        try {
            pmrp.set_allocation_limit(exceptionCounter);
            codeBlock(pmrp);
            pmrp.set_allocation_limit(-1);
            return;
        } catch (const test_resource_exception& e) {
            if (e.originating_resource() != &pmrp) {
                std::printf("\t*** test_resource_exception"
                            " from unexpected test resource: %p %.*s ***\n",
                            e.originating_resource(),
                            static_cast<int>(
                                         e.originating_resource()->name().length()),
                            e.originating_resource()->name().data());
                throw;
            }
            else if (pmrp.is_verbose()) {
                std::printf("\t*** test_resource_exception: "
                            "alloc limit = %lld, last alloc size = %zu, "
                            "align = %zu ***\n",
                            exceptionCounter,
                            e.size(),
                            e.alignment());
            }
        }
    }
}


class [[maybe_unused]] default_resource_guard {
    std::pmr::memory_resource * m_old_resource;

  public:
    explicit default_resource_guard(std::pmr::memory_resource * newDefault)
    {
        assert(newDefault != nullptr);
        m_old_resource = std::pmr::set_default_resource(newDefault);
    }

    default_resource_guard(const default_resource_guard&) = delete;

    default_resource_guard& operator=(const default_resource_guard&) = delete;

    ~default_resource_guard()
    {
        std::pmr::set_default_resource(m_old_resource);
    }
};

/*
template<class ValueType = byte>
class polymorphic_allocator_P0339R5 : public std::polymorphic_allocator<ValueType> {
    // See Pablo Halpern, Dietmar Kuehl (2018). P0339R5 polymorphic_allocator<>
    // as a vocabulary type
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0339r5.pdf

    std::pmr::memory_resource *_Resource()
    {
        return *reinterpret_cast<std::pmr::memory_resource **>(this);
    }

public:
    template<class>
        friend class polymorphic_allocator_P0339R4;

    using value_type = ValueType;

    using std::polymorphic_allocator<ValueType>::polymorphic_allocator;

    polymorphic_allocator_P0339R5&
        operator=(const polymorphic_allocator_P0339R5&) = delete;

    [[nodiscard]]
    void *allocate_bytes(const std::size_t bytes,
                         const std::size_t alignment = alignof(max_align_t))
    {
        return (_Resource()->allocate(bytes, alignment));
    }

    void deallocate_bytes(void * const      ptr,
                          const std::size_t bytes,
                          const std::size_t alignment = alignof(max_align_t))
    {
        return (_Resource()->deallocate(ptr, bytes, alignment));
    }

    template <class ObjectType>
    [[nodiscard]] ObjectType *allocate_object(const std::size_t count = 1)
    {
        return  static_cast<ObjectType *>(
              allocate_bytes(count * sizeof(ObjectType), alignof(ObjectType)));
    }

    template <class ObjectType>
    void deallocate_object(ObjectType *ptr, const std::size_t count = 1)
    {
        deallocate_bytes(ptr, count * sizeof(ObjectType), alignof(ObjectType));
    }

    template <class ObjectType, class... ArgTypes>
    [[nodiscard]] ObjectType * new_object(ArgTypes&&... args)
    {
        void *ptr = allocate_object<ObjectType>();
        try
        {
            construct(ptr, std::forward<ArgTypes>(args)...);
        }
        catch (...)
        {
            _Resource()->deallocate(ptr,
                                    sizeof(ObjectType),
                                    alignof(ObjectType));
            throw;
        }
    }

    template <class ObjectType>
    void delete_object(ObjectType *ptr)
    {
        destroy(ptr);
        deallocate_object(ptr);
    }
};
*/
} // close namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2019 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
