// test_resource.cpp                                                  -*-C++-*-
#include <pmr/test_resource.hpp>

#include <algorithm>  // for min
#include <cassert>    // for assert
#include <cstdio>     // print messages
#include <cstddef>    // byte
#include <cstdlib>    // abort
#include <cstring>    // memset

namespace beman::pmr {

namespace {

static const unsigned int allocatedMemoryPattern = 0xDEADBEEF;
    // magic number identifying memory allocated by this resource

static const unsigned int deallocatedMemoryPattern = 0xDEADF00D;
    // 2nd magic number written over other magic number upon deallocation

static const std::byte scribbledMemoryByte{ 0xA5 };  // byte used to scribble
                                                     // deallocated memory

static const std::byte paddedMemoryByte{ 0xB1 };     // byte used to write over
                                                     // newly-allocated memory
                                                     // and padding

static const std::size_t paddingSize = alignof(max_align_t);
    // size of the padding before and after the user segment

struct Link {
    // This 'struct' holds pointers to the next and preceding allocated
    // memory block in the allocated memory block list.

    long long  m_index;  // index of this allocation
    Link      *m_next;   // next 'Link' pointer
    Link      *m_prev;   // previous 'Link' pointer
};

struct alignas(std::max_align_t) Padding {
    // This struct just make 'Header' readable.

    std::byte m_padding[paddingSize];
};

struct Header {
    // This 'struct' defines the data preceding the user segment.

    unsigned int  m_magic_number;  // allocated/deallocated/other identifier

    std::size_t   m_bytes;         // number of available bytes in this block

    std::size_t   m_alignment;     // the allocation alignment

    long long     m_index;         // index of this memory allocation

    Link         *m_address;       // address of block in linked list

    void         *m_pmr;           // address of current PMR

    Padding       m_padding;       // padding -- guaranteed to extend to the
                                   // end of the struct
};

union AlignedHeader {
    // Maximally-aligned raw buffer big enough for a Header.

    Header           m_object;
    std::max_align_t m_alignment;
};

}  // close unnamed namespace

static
void formatBlock(void *address, std::size_t length)
    // Format in hex to 'stdout', a block of memory starting at the specified
    // starting 'address' of the specified 'length' (in bytes).  Each line of
    // formatted output will have a maximum of 16 bytes per line, where each
    // line starts with the address of that 16-byte chunk.
{
    std::byte *addr    = reinterpret_cast<std::byte *>(address);
    std::byte *endAddr = addr + length;

    for (int i = 0; addr < endAddr; ++i) {
        if (0 == i % 4) {
            if (i) {
                std::printf("\n");
            }
            std::printf("%p:\t", static_cast<void *>(addr));
        }
        else {
            std::printf("  ");
        }

        for (int j = 0; j < 4 && addr < endAddr; ++j) {
            std::printf("%02x ", *addr);
            ++addr;
        }
    }

    std::printf("\n");
}

static
void formatInvalidMemoryBlock(AlignedHeader *address,
                              std::size_t    deallocatedBytes,
                              std::size_t    deallocatedAlignment,
                              test_resource *allocator,
                              int            underrunBy,
                              int            overrunBy)
    // Format the contents of the presumably invalid memory block at the
    // specified 'address' to 'stdout', using the specified 'allocator',
    // 'underrunBy', and 'overrunBy' information.  A suitable error message,
    // if appropriate, is printed first, followed by a block of memory
    // indicating the header and any extra padding appropriate for the current
    // platform.  Finally, the first 64 bytes of memory of the "payload"
    // portion of the allocated memory is printed (regardless of the amount of
    // memory that was requested).
{
    unsigned int  magicNumber = address->m_object.m_magic_number;
    std::size_t   numBytes    = address->m_object.m_bytes;
    std::size_t   alignment   = address->m_object.m_alignment;
    std::byte    *payload     = reinterpret_cast<std::byte *>(address + 1);

    if (allocatedMemoryPattern != magicNumber) {
        if (deallocatedMemoryPattern == magicNumber) {
            std::printf("*** Deallocating previously deallocated memory at %p."
                        " ***\n",
                        static_cast<void *>(payload));
        }
        else {
            std::printf("*** Invalid magic number 0x%08x at address %p. ***\n",
                        magicNumber,
                        static_cast<void *>(payload));
        }
    }
    else {
        if (numBytes <= 0) {
            std::printf("*** Invalid (non-positive) byte count %zu at address "
                        "%p. *** \n",
                        numBytes,
                        static_cast<void *>(payload));
        }
        if (deallocatedBytes != address->m_object.m_bytes) {
            std::printf("*** Freeing segment at %p using wrong size (%zu vs. "
                        "%zu). ***\n",
                        static_cast<void *>(payload),
                        deallocatedBytes,
                        numBytes);
        }
        if (deallocatedAlignment != address->m_object.m_alignment) {
            std::printf("*** Freeing segment at %p using wrong alignment (%zu "
                        "vs. %zu). ***\n",
                        static_cast<void *>(payload),
                        deallocatedAlignment,
                        alignment);
        }
        if (allocator != address->m_object.m_pmr) {
            std::printf("*** Freeing segment at %p from wrong allocator. "
                        "***\n",
                        static_cast<void *>(payload));
        }
        if (underrunBy) {
            std::printf("*** Memory corrupted at %d bytes before %zu byte "
                        "segment at %p. ***\n",
                        underrunBy,
                        numBytes,
                        static_cast<void *>(payload));

            std::printf("Pad area before user segment:\n");
            formatBlock(payload - paddingSize, paddingSize);
        }
        if (overrunBy) {
            std::printf("*** Memory corrupted at %d bytes after %zu byte "
                        "segment at %p. ***\n",
                        overrunBy,
                        numBytes,
                        static_cast<void *>(payload));

            std::printf("Pad area after user segment:\n");
            formatBlock(payload + numBytes, paddingSize);
        }
    }

    std::printf("Header:\n");
    formatBlock(address, sizeof *address);
    std::printf("User segment:\n");
    formatBlock(payload, std::min<std::size_t>(64, numBytes));
}

static
void formatBadBytesForNullptr(std::size_t deallocatedBytes,
                              std::size_t deallocatedAlignment)
    // Print an error message to standard output that describes that a
    // 'nullptr' with a non-zero size (bytes) was attempted to be deallocated.
    // The error message should contain the specified 'deallocatedBytes',
    // 'deallocatedAlignment', and 'allocator' address.  The behavior is
    // undefined unless 'deallocatedBytes != 0'.
{
    assert(deallocatedBytes != 0);

    std::printf("*** Freeing a nullptr using non-zero size (%zu) with "
                "alignment (%zu). ***\n",
                deallocatedBytes,
                deallocatedAlignment);
}

struct test_resource_list {
    // This 'struct' stores a head 'Link' and a tail 'Link' for list
    // manipulation.

    Link *m_head_p;  // address of first link in list (or 'nullptr')
    Link *m_tail_p;  // address of last link in list (or 'nullptr')
};

static
Link *removeLink(test_resource_list *list, Link *link)
    // Remove the specified 'link' from the specified 'allocatedList'.  Return
    // the address of the removed link.  The behavior is undefined unless
    // 'allocatedList' and 'link' are non-zero.  Note that the tail pointer of
    // the 'allocatedList' will be updated if the 'Link' removed is the tail
    // itself, and the head pointer of the 'allocatedList' will be updated if
    // the link removed is the head itself.
{

    if (link == list->m_tail_p) {
        list->m_tail_p  = link->m_prev;
    }
    else {
        link->m_next->m_prev = link->m_prev;
    }

    if (link == list->m_head_p) {
        list->m_head_p  = link->m_next;
    }
    else {
        link->m_prev->m_next = link->m_next;
    }

    return link;
}

static
Link *addLink(test_resource_list        *list,
              long long                  index,
              std::pmr::memory_resource *pmrp)
    // Add to the specified 'list' a 'Link' having the specified 'index', and
    // using the specified 'pmrp' polymopric memory resource to supply memory;
    // also update the tail pointer of the 'list'.  Return the address of the
    // allocated 'Link'.  Note that the head pointer of the 'list' will be
    // updated if the 'list' is initially empty.
{
    Link *link = static_cast<Link *>(pmrp->allocate(sizeof(Link)));

    assert(nullptr != link);  // Ensure 'allocate' returned memory.

    link->m_next = nullptr;
    link->m_index  = index;

    if (!list->m_head_p) {
        list->m_head_p = link;
        list->m_tail_p = link;
        link->m_prev   = nullptr;
    }
    else {
        list->m_tail_p->m_next = link;
        link->m_prev           = list->m_tail_p;
        list->m_tail_p         = link;
    }

    return link;
}

static
void printList(const test_resource_list& list)
    // Print the indices of all 'Link' objects currently in the specified
    // 'allocatedList'.
{
    const Link *pLink = list.m_head_p;

    while (pLink) {
        for (int i = 0; i < 8 && pLink; ++i) {
            printf("%lld\t", pLink->m_index);
            pLink = pLink->m_next;
        }

        // The space after the '\n' is needed to align these indices properly
        // with the current output.

        printf("\n ");
    }
}

static
std::pmr::memory_resource* local_malloc_free_resource()
{
    struct malloc_free_resource : std::pmr::memory_resource {
        [[nodiscard]] void *
            do_allocate(std::size_t bytes, std::size_t alignment) override
        {
            // While this class is local, there is no need to check the
            // alignment, as we never ask for overaligned memory
            //if (alignment > alignof(max_align_t)) {
            //    throw bad_alloc();
            //}
            void *rv = std::malloc(bytes);
            if (nullptr == rv) {
                throw std::bad_alloc();
            }
            return rv;
        }

        void do_deallocate(void                         *p,
                           [[maybe_unused]] std::size_t  bytes,
                           [[maybe_unused]] std::size_t  alignment) override
        {
            std::free(p);
        }

        bool do_is_equal(const memory_resource& that) const noexcept override
        {
            return nullptr != dynamic_cast<const malloc_free_resource*>(&that);
        }
    };

    static malloc_free_resource instance;
    return &instance;
}

test_resource::test_resource(std::pmr::memory_resource *pmrp)
: test_resource(false, pmrp)
{
}

test_resource::test_resource(std::string_view           name,
                             std::pmr::memory_resource *pmrp)
: test_resource(name, false, pmrp)
{
}

test_resource::test_resource(const char *name, std::pmr::memory_resource *pmrp)
: test_resource(std::string_view{name}, false, pmrp)
{
}

test_resource::test_resource(bool verbose, memory_resource *pmrp)
: test_resource(std::string_view{}, verbose, pmrp)
{
}

test_resource::test_resource(const char                *name,
                             bool                       verbose,
                             std::pmr::memory_resource *pmrp)
: test_resource(std::string_view(name), verbose, pmrp)
{
}

test_resource::test_resource(std::string_view           name,
                             bool                       verbose,
                             std::pmr::memory_resource *pmrp)
: m_name(name, local_malloc_free_resource())
, m_verbose_flag(verbose)
, m_pmr(pmrp ? pmrp : local_malloc_free_resource())
{
    m_list = (test_resource_list *)m_pmr->allocate(sizeof(test_resource_list));
    m_list->m_head_p = nullptr;
    m_list->m_tail_p = nullptr;
}

test_resource::~test_resource()
{
    if (is_verbose()) {
        print();
    }

    Link *link_p = m_list->m_head_p;
    while (link_p) {
        Link *linkToFree = link_p;
        link_p = link_p->m_next;
        m_pmr->deallocate(linkToFree, sizeof(Link), alignof(Link));
    }
    m_list->m_head_p = nullptr;
    m_list->m_tail_p = nullptr;
    m_pmr->deallocate(m_list,
                      sizeof(test_resource_list),
                      alignof(test_resource_list));

    if (!is_quiet()) {
        if (bytes_in_use() || blocks_in_use()) {
            std::printf("MEMORY_LEAK");
            if (!m_name.empty()) {
                std::printf(" from %.*s",
                            static_cast<int>(m_name.length()),
                            m_name.data());
            }
            std::printf(":\n  Number of blocks in use = %lld\n"
                        "   Number of bytes in use = %lld\n",
                        blocks_in_use(), bytes_in_use());

            if (!is_no_abort()) {
                std::abort();                                          // ABORT
            }
        }
    }
}

void *test_resource::do_allocate(std::size_t bytes, std::size_t alignment)
{
    std::lock_guard guard{ m_lock };

    m_allocate_calls.fetch_add(1, std::memory_order_relaxed);

    if (alignment > alignof(std::max_align_t)) {
        // Over-aligned allocations are not currently supported.
        throw std::bad_alloc();
    }

    if (0 <= allocation_limit()) {
        if (0 > m_allocation_limit.fetch_add(-1,
                                             std::memory_order_relaxed) - 1) {
            throw test_resource_exception(this, bytes, alignment);
        }
    }

    // The upstream resource will throw bad_alloc if it cannot fulfill
    AlignedHeader *head = static_cast<AlignedHeader *>(m_pmr->allocate(
                                   sizeof(AlignedHeader) + bytes + paddingSize,
                                   alignof(AlignedHeader)));

    long long allocationIndex = 
                         m_allocations.fetch_add(1, std::memory_order_relaxed);

    m_last_allocated_num_bytes.store(static_cast<long long>(bytes),
                                     std::memory_order_relaxed);
    m_last_allocated_alignment.store(static_cast<long long>(alignment),
                                     std::memory_order_relaxed);

    // Note that we don't initialize the user portion of the segment because
    // that would undermine Purify's 'UMR: uninitialized memory read' checking.

    std::memset((char *)(head + 1) - paddingSize,
                std::to_integer<unsigned char>(paddedMemoryByte), paddingSize);
    std::memset((char *)(head + 1) + bytes,
                std::to_integer<unsigned char>(paddedMemoryByte), paddingSize);

    head->m_object.m_bytes        = bytes;
    head->m_object.m_alignment    = alignment;
    head->m_object.m_magic_number = allocatedMemoryPattern;
    head->m_object.m_index        = allocationIndex;

    m_blocks_in_use.fetch_add(1, std::memory_order_relaxed);
    if (max_blocks() < blocks_in_use()) {
        m_max_blocks.store(blocks_in_use(), std::memory_order_relaxed);
    }
    m_total_blocks.fetch_add(1, std::memory_order_relaxed);

    m_bytes_in_use.fetch_add(static_cast<long long>(bytes),
                             std::memory_order_relaxed);
    if (max_bytes() < bytes_in_use()) {
        m_max_bytes.store(bytes_in_use(), std::memory_order_relaxed);
    }
    m_total_bytes.fetch_add(static_cast<long long>(bytes),
                            std::memory_order_relaxed);

    Link *link = addLink(m_list, allocationIndex, m_pmr);
    head->m_object.m_address = link;
    head->m_object.m_pmr     = this;

    void *address = ++head;

    m_last_allocated_address.store(address, std::memory_order_relaxed);

    if (is_verbose()) {

        // In verbose mode, print a message to 'stdout' -- e.g.,
        //..
        //  TestAllocator global [25]: Allocated 128 bytes at 0xc3a281a8.
        //..

        std::printf("test_resource");

        if (!m_name.empty()) {
            std::printf(" %.*s",
                        static_cast<int>(m_name.length()), m_name.data());
        }

        std::printf(" [%lld]: Allocated %zu byte%s(aligned %zu) at %p.\n",
                    allocationIndex,
                    bytes,
                    1 == bytes ? " " : "s ",
                    alignment,
                    address);

        std::fflush(stdout);
    }

    return address;
}

void test_resource::do_deallocate(void        *p,
                                  std::size_t  bytes,
                                  std::size_t  alignment)
{
    std::lock_guard guard{ m_lock };

    m_deallocate_calls.fetch_add(1, std::memory_order_relaxed);
    m_last_deallocated_address.store(p, std::memory_order_relaxed);

    AlignedHeader *head = (AlignedHeader *)p - 1;

    bool miscError  = false;
    bool paramError = false;

    size_t     size            = 0;
    long long  allocationIndex = -1;

    // The following checks are done deliberately in the order shown to avoid a
    // possible bus error when attempting to read a misaligned 64-bit integer,
    // which can happen if an invalid address is passed to this method.  If the
    // address of the memory being deallocated is misaligned, it is very likely
    // that 'm_magic_number_' will not match the expected value, and so we will
    // skip the reading of 'm_bytes_' (a 64-bit integer).

    if (allocatedMemoryPattern != head->m_object.m_magic_number) {
        miscError = true;
    }
    else if (this != head->m_object.m_pmr) {
        miscError = true;
    }
    else {
        size            = head->m_object.m_bytes;
        allocationIndex = head->m_object.m_index;
    }

    // If there is evidence of corruption, this memory may have already been
    // freed.  On some platforms (but not others), the 'free' function will
    // scribble freed memory.  To get uniform behavior for test drivers, we
    // deliberately don't check over/underruns if 'miscError' is 'true'.

    int overrunBy  = 0;
    int underrunBy = 0;

    if (!miscError) {
        std::byte *pcBegin;
        std::byte *pcEnd;

        // Check the padding before the segment.  Go backwards so we will
        // report the trashed byte nearest the segment.

        pcBegin = (std::byte *)p - 1;
        pcEnd   = (std::byte *)&head->m_object.m_padding;

        for (std::byte *pc = pcBegin; pcEnd <= pc; --pc) {
            if (paddedMemoryByte != *pc) {
                underrunBy = static_cast<int>(pcBegin + 1 - pc);
                break;
            }
        }

        if (!underrunBy) {
            // Check the padding after the segment.

            std::byte *tail = (std::byte *)p + size;
            pcBegin = tail;
            pcEnd = tail + paddingSize;
            for (std::byte *pc = pcBegin; pc < pcEnd; ++pc) {
                if (paddedMemoryByte != *pc) {
                    overrunBy = static_cast<int>(pc + 1 - pcBegin);
                    break;
                }
            }
        }

        if (bytes != size || alignment != head->m_object.m_alignment) {
            paramError = true;
        }
    }

    // Now check for corrupted memory block and cross allocation.

    if (!miscError && !overrunBy && !underrunBy &&!paramError) {
        m_pmr->deallocate(removeLink(m_list,
                                     head->m_object.m_address),
                          sizeof(Link), alignof(Link));
    }
    else { // Any error, count it, report it
        if (miscError) {
            m_mismatches.fetch_add(1, std::memory_order_relaxed);
        }
        if (paramError) {
            m_bad_deallocate_params.fetch_add(1, std::memory_order_relaxed);
        }
        if (overrunBy || underrunBy) {
            m_bounds_errors.fetch_add(1, std::memory_order_relaxed);
        }

        if (is_quiet()) {
            return;                                                   // RETURN
        }
        else {
            formatInvalidMemoryBlock(head, bytes, alignment,
                                     this, underrunBy, overrunBy);
            if (is_no_abort()) {
                return;                                               // RETURN
            }
            else {
                std::abort();                                          // ABORT
            }
        }
    }

    // At this point we know (almost) for sure that the memory block is
    // currently allocated from this object.  We now proceed to update our
    // statistics, stamp the block's header as deallocated, scribble over its
    // payload, and give it back to the underlying allocator supplied at
    // construction.  In verbose mode, we also report the deallocation event to
    // 'stdout'.

    m_last_deallocated_num_bytes.store(static_cast<long long>(size),
                                       std::memory_order_relaxed);
    m_last_deallocated_alignment.store(static_cast<long long>(alignment),
                                       std::memory_order_relaxed);

    m_blocks_in_use.fetch_add(-1, std::memory_order_relaxed);

    m_bytes_in_use.fetch_add(-static_cast<long long>(size),
                             std::memory_order_relaxed);

    head->m_object.m_magic_number = deallocatedMemoryPattern;

    std::memset(p, static_cast<int>(scribbledMemoryByte), size);

    if (is_verbose()) {

        // In verbose mode, print a message to 'stdout' -- e.g.,
        //..
        //  TestAllocator local [245]: Deallocated 1 byte at 0x3c1b2740.
        //..

        std::printf("test_resource");

        if (!m_name.empty()) {
            std::printf(" %.*s",
                        static_cast<int>(m_name.length()), m_name.data());
        }

        std::printf(" [%lld]: Deallocated %zu byte%s(aligned %zu) at %p.\n",
                    allocationIndex,
                    size,
                    1 == size ? " " : "s ",
                    alignment,
                    p);

        std::fflush(stdout);
    }

    m_deallocations.fetch_add(1, std::memory_order_relaxed);
    m_pmr->deallocate(head,
                      sizeof(AlignedHeader) + size + paddingSize,
                      alignof(AlignedHeader));
}

bool test_resource::do_is_equal(const std::pmr::memory_resource& that)
                                                                 const noexcept
{
    return this == &that;
}

void test_resource::print() const noexcept
{
    std::lock_guard guard{ m_lock };

    if (!m_name.empty()) {
        std::printf("\n"
                    "==================================================\n"
                    "                TEST RESOURCE %.*s STATE\n"
                    "--------------------------------------------------\n",
                    static_cast<int>(m_name.length()), m_name.data());
    }
    else {
        std::printf("\n"
                    "==================================================\n"
                    "                TEST RESOURCE STATE\n"
                    "--------------------------------------------------\n");
    }

    std::printf("        Category\tBlocks\tBytes\n"
                "        --------\t------\t-----\n"
                "          IN USE\t%lld\t%lld\n"
                "             MAX\t%lld\t%lld\n"
                "           TOTAL\t%lld\t%lld\n"
                "      MISMATCHES\t%lld\n"
                "   BOUNDS ERRORS\t%lld\n"
                "   PARAM. ERRORS\t%lld\n"
                "--------------------------------------------------\n",
                blocks_in_use(), bytes_in_use(),
                max_blocks(),    max_bytes(),
                total_blocks(),  total_bytes(),
                mismatches(),    bounds_errors(),
                bad_deallocate_params());

    if (m_list->m_head_p) {
        std::printf(" Indices of Outstanding Memory Allocations:\n ");
        printList(*m_list);
    }
    std::fflush(stdout);
}

long long test_resource::status() const noexcept
{
    static const int memoryLeak = -1;
    static const int success = 0;

    std::lock_guard guard{ m_lock };

    const long long numErrors = mismatches() + bounds_errors() +
                                bad_deallocate_params();

    if (numErrors > 0) {
        return static_cast<int>(numErrors);                           // RETURN
    }
    else if (has_allocations()) {
        return memoryLeak;                                            // RETURN
    }
    else {
        return success;                                               // RETURN
    }
}

}  // close namespace

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
