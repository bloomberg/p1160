// test_resource.cpp                                                  -*-C++-*-
#include <memory_resource_p1160>

#include <algorithm>  // for min
#include <cassert>    // for assert
#include <cstdio>     // print messages
#include <cstddef>    // byte
#include <cstdlib>    // abort
#include <cstring>    // memset

namespace std::pmr {

namespace {

static const unsigned int allocatedMemoryPattern = 0xDEADBEEF;
    // magic number identifying memory allocated by this resource

static const unsigned int deallocatedMemoryPattern = 0xDEADF00D;
    // 2nd magic number written over other magic number upon deallocation

static const byte scribbledMemoryByte{ 0xA5 };  // byte used to scribble
                                                // deallocated memory

static const byte paddedMemoryByte{ 0xB1 };     // byte used to write over
                                                // newly-allocated memory and
                                                // padding

static const size_t paddingSize = alignof(max_align_t);
    // size of the padding before and after the user segment

struct Link {
    // This 'struct' holds pointers to the next and preceding allocated
    // memory block in the allocated memory block list.

    long long  m_index_;  // index of this allocation
    Link      *m_next_;   // next 'Link' pointer
    Link      *m_prev_;   // previous 'Link' pointer
};

struct alignas(std::max_align_t) Padding {
    // This struct just make 'Header' readable.

    byte m_padding_[paddingSize];
};

struct Header {
    // This 'struct' defines the data preceding the user segment.

    unsigned int  m_magic_number_;  // allocated/deallocated/other identifier

    size_t        m_bytes_;         // number of available bytes in this block

    size_t        m_alignment_;     // the allocation alignment

    long long     m_index_;         // index of this memory allocation

    Link         *m_address_;       // address of block in linked list

    void         *m_pmr_;           // address of current PMR

    Padding       m_padding_;       // padding -- guaranteed to extend to the
                                    // end of the struct
};

union AlignedHeader {
    // Maximally-aligned raw buffer big enough for a Header.

    Header      m_object_;
    max_align_t m_alignment_;
};

}  // close unnamed namespace

static
void formatBlock(void *address, std::size_t length)
    // Format in hex to 'stdout', a block of memory starting at the specified
    // starting 'address' of the specified 'length' (in bytes).  Each line of
    // formatted output will have a maximum of 16 bytes per line, where each
    // line starts with the address of that 16-byte chunk.
{
    byte *addr    = reinterpret_cast<byte *>(address);
    byte *endAddr = addr + length;

    for (int i = 0; addr < endAddr; ++i) {
        if (0 == i % 4) {
            if (i) {
                printf("\n");
            }
            printf("%p:\t", static_cast<void *>(addr));
        }
        else {
            printf("  ");
        }

        for (int j = 0; j < 4 && addr < endAddr; ++j) {
            printf("%02x ", *addr);
            ++addr;
        }
    }

    printf("\n");
}

static
void formatInvalidMemoryBlock(AlignedHeader *address,
                              size_t         deallocatedBytes,
                              size_t         deallocatedAlignment,
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
    unsigned int  magicNumber = address->m_object_.m_magic_number_;
    size_t        numBytes    = address->m_object_.m_bytes_;
    size_t        alignment   = address->m_object_.m_alignment_;
    byte         *payload     = reinterpret_cast<byte *>(address + 1);

    if (allocatedMemoryPattern != magicNumber) {
        if (deallocatedMemoryPattern == magicNumber) {
            printf("*** Deallocating previously deallocated memory at %p."
                   " ***\n",
                   static_cast<void *>(payload));
        }
        else {
            printf("*** Invalid magic number 0x%08x at address %p. ***\n",
                   magicNumber,
                   static_cast<void *>(payload));
        }
    }
    else {
        if (numBytes <= 0) {
            printf("*** Invalid (non-positive) byte count %zu at address %p. "
                   "*** \n",
                   numBytes,
                   static_cast<void *>(payload));
        }
        if (deallocatedBytes != address->m_object_.m_bytes_) {
            printf("*** Freeing segment at %p using wrong size (%zu vs. %zu). "
                   "***\n",
                   static_cast<void *>(payload),
                   deallocatedBytes,
                   numBytes);
        }
        if (deallocatedAlignment != address->m_object_.m_alignment_) {
            printf("*** Freeing segment at %p using wrong alignment (%zu vs. "
                   "%zu). ***\n",
                   static_cast<void *>(payload),
                   deallocatedAlignment,
                   alignment);
        }
        if (allocator != address->m_object_.m_pmr_) {
            printf("*** Freeing segment at %p from wrong allocator. ***\n",
                   static_cast<void *>(payload));
        }
        if (underrunBy) {
            printf("*** Memory corrupted at %d bytes before %zu byte segment "
                   "at %p. ***\n",
                   underrunBy,
                   numBytes,
                   static_cast<void *>(payload));

            printf("Pad area before user segment:\n");
            formatBlock(payload - paddingSize, paddingSize);
        }
        if (overrunBy) {
            printf("*** Memory corrupted at %d bytes after %zu byte segment "
                   "at %p. ***\n",
                   overrunBy,
                   numBytes,
                   static_cast<void *>(payload));

            printf("Pad area after user segment:\n");
            formatBlock(payload + numBytes, paddingSize);
        }
    }

    printf("Header:\n");
    formatBlock(address, sizeof *address);
    printf("User segment:\n");
    formatBlock(payload, min<std::size_t>(64, numBytes));
}

static
void formatBadBytesForNullptr(size_t         deallocatedBytes,
                              size_t         deallocatedAlignment)
    // Print an error message to standard output that describes that a
    // 'nullptr' with a non-zero size (bytes) was attempted to be deallocated.
    // The error message should contain the specified 'deallocatedBytes',
    // 'deallocatedAlignment', and 'allocator' address.  The behavior is
    // undefined unless 'deallocatedBytes != 0'.
{
    assert(deallocatedBytes != 0);

    printf("*** Freeing a nullptr using non-zero size (%zu) with alignment "
           "(%zu). ***\n", deallocatedBytes, deallocatedAlignment);
}

struct test_resource_list {
    // This 'struct' stores a head 'Link' and a tail 'Link' for list
    // manipulation.

    Link *d_head_p;  // address of first link in list (or 'nullptr')
    Link *d_tail_p;  // address of last link in list (or 'nullptr')
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

    if (link == list->d_tail_p) {
        list->d_tail_p  = link->m_prev_;
    }
    else {
        link->m_next_->m_prev_ = link->m_prev_;
    }

    if (link == list->d_head_p) {
        list->d_head_p  = link->m_next_;
    }
    else {
        link->m_prev_->m_next_ = link->m_next_;
    }

    return link;
}

static
Link *addLink(test_resource_list *list,
              long long           index,
              memory_resource    *pmrp)
    // Add to the specified 'list' a 'Link' having the specified 'index', and
    // using the specified 'pmrp' polymopric memory resource to supply memory;
    // also update the tail pointer of the 'list'.  Return the address of the
    // allocated 'Link'.  Note that the head pointer of the 'list' will be
    // updated if the 'list' is initially empty.
{
    Link *link = static_cast<Link *>(pmrp->allocate(sizeof(Link)));

    assert(nullptr != link);  // Ensure 'allocate' returned memory.

    link->m_next_ = nullptr;
    link->m_index_  = index;

    if (!list->d_head_p) {
        list->d_head_p = link;
        list->d_tail_p = link;
        link->m_prev_ = nullptr;
    }
    else {
        list->d_tail_p->m_next_ = link;
        link->m_prev_ = list->d_tail_p;
        list->d_tail_p = link;
    }

    return link;
}

static
void printList(const test_resource_list& list)
    // Print the indices of all 'Link' objects currently in the specified
    // 'allocatedList'.
{
    const Link *pLink = list.d_head_p;

    while (pLink) {
        for (int i = 0; i < 8 && pLink; ++i) {
            printf("%lld\t", pLink->m_index_);
            pLink = pLink->m_next_;
        }

        // The space after the '\n' is needed to align these indices properly
        // with the current output.

        printf("\n ");
    }
}

test_resource::test_resource()
: test_resource(string_view{}, false)
{
}

test_resource::test_resource(memory_resource *pmrp)
: test_resource(string_view{}, false, pmrp)
{
}

test_resource::test_resource(string_view name)
: test_resource(name, false)
{
}

test_resource::test_resource(const char *name)
: test_resource(string_view(name), false)
{
}

test_resource::test_resource(bool verbose)
: test_resource(string_view{}, verbose)
{
}

test_resource::test_resource(string_view name, memory_resource *pmrp)
: test_resource(name, false, pmrp)
{
}

test_resource::test_resource(const char *name, memory_resource *pmrp)
: test_resource(string_view(name), false, pmrp)
{
}

test_resource::test_resource(bool verbose, memory_resource *pmrp)
: test_resource(string_view{}, verbose, pmrp)
{
}

test_resource::test_resource(string_view name, bool verbose)
: test_resource(name, verbose, new_delete_resource())
{
}

test_resource::test_resource(const char *name, bool verbose)
: test_resource(string_view(name), verbose, new_delete_resource())
{
}

test_resource::test_resource(string_view      name,
                             bool             verbose,
                             memory_resource *pmrp)
: m_name_(name)
, m_verbose_flag_(verbose)
, m_pmr_(pmrp)
{
    m_list_ = (test_resource_list *)m_pmr_->allocate(
                                                   sizeof(test_resource_list));
    m_list_->d_head_p = nullptr;
    m_list_->d_tail_p = nullptr;
}

test_resource::test_resource(const char      *name,
                             bool             verbose,
                             memory_resource *pmrp)
: test_resource(string_view(name), verbose, pmrp)
{
}

test_resource::~test_resource()
{
    if (is_verbose()) {
        print();
    }

    Link *link_p = m_list_->d_head_p;
    while (link_p) {
        Link *linkToFree = link_p;
        link_p = link_p->m_next_;
        m_pmr_->deallocate(linkToFree, sizeof(Link), alignof(Link));
    }
    m_list_->d_head_p = nullptr;
    m_list_->d_tail_p = nullptr;
    m_pmr_->deallocate(m_list_,
                       sizeof(test_resource_list),
                       alignof(test_resource_list));

    if (!is_quiet()) {
        if (bytes_in_use() || blocks_in_use()) {
            printf("MEMORY_LEAK");
            if (!m_name_.empty()) {
                printf(" from %.*s",
                       static_cast<int>(m_name_.length()), m_name_.data());
            }
            printf(":\n  Number of blocks in use = %lld\n"
                   "   Number of bytes in use = %lld\n",
                   blocks_in_use(), bytes_in_use());

            if (!is_no_abort()) {
                std::abort();                                          // ABORT
            }
        }
    }
}

void *test_resource::do_allocate(size_t bytes, size_t alignment)
{
    lock_guard guard{ m_lock_ };

    long long allocationIndex = m_allocations_.fetch_add(1,
                                                         memory_order_relaxed);

    if (alignment > alignof(max_align_t)) {
        // Over-aligned allocations are not currently supported.
        throw bad_alloc();
    }

    if (0 <= allocation_limit()) {
        if (0 > m_allocation_limit_.fetch_add(-1, memory_order_relaxed) - 1) {
            throw test_resource_exception(this, bytes, alignment);
        }
    }

    if (0 == bytes) {
        m_last_allocated_num_bytes_.store(static_cast<long long>(0),
                                          memory_order_relaxed);
        m_last_allocated_alignment_.store(static_cast<long long>(alignment),
                                          memory_order_relaxed);
        m_last_allocated_address_.store(nullptr, memory_order_relaxed);
        return nullptr;                                               // RETURN
    }

    AlignedHeader *head = (AlignedHeader *)m_pmr_->allocate(
                                  sizeof(AlignedHeader) + bytes + paddingSize);
    if (!head) {
        // We cannot satisfy this request.  Throw 'std::bad_alloc'.

        throw bad_alloc();
    }

    m_last_allocated_num_bytes_.store(static_cast<long long>(bytes),
                                      memory_order_relaxed);
    m_last_allocated_alignment_.store(static_cast<long long>(alignment),
                                      memory_order_relaxed);

    // Note that we don't initialize the user portion of the segment because
    // that would undermine Purify's 'UMR: uninitialized memory read' checking.

    std::memset((char *)(head + 1) - paddingSize,
                to_integer<unsigned char>(paddedMemoryByte), paddingSize);
    std::memset((char *)(head + 1) + bytes,
                to_integer<unsigned char>(paddedMemoryByte), paddingSize);

    head->m_object_.m_bytes_        = bytes;
    head->m_object_.m_alignment_    = alignment;
    head->m_object_.m_magic_number_ = allocatedMemoryPattern;
    head->m_object_.m_index_        = allocationIndex;

    m_blocks_in_use_.fetch_add(1, memory_order_relaxed);
    if (max_blocks() < blocks_in_use()) {
        m_max_blocks_.store(blocks_in_use(), memory_order_relaxed);
    }
    m_total_blocks_.fetch_add(1, memory_order_relaxed);

    m_bytes_in_use_.fetch_add(static_cast<long long>(bytes),
                              memory_order_relaxed);
    if (max_bytes() < bytes_in_use()) {
        m_max_bytes_.store(bytes_in_use(), memory_order_relaxed);
    }
    m_total_bytes_.fetch_add(static_cast<long long>(bytes),
                             memory_order_relaxed);

    Link *link = addLink(m_list_, allocationIndex, m_pmr_);
    head->m_object_.m_address_ = link;
    head->m_object_.m_pmr_      = this;

    void *address = ++head;

    m_last_allocated_address_.store(address, memory_order_relaxed);

    if (is_verbose()) {

        // In verbose mode, print a message to 'stdout' -- e.g.,
        //..
        //  TestAllocator global [25]: Allocated 128 bytes at 0xc3a281a8.
        //..

        printf("test_resource");

        if (!m_name_.empty()) {
            printf(" %.*s",
                   static_cast<int>(m_name_.length()), m_name_.data());
        }

        printf(" [%lld]: Allocated %zu byte%s(aligned %zu) at %p.\n",
               allocationIndex,
               bytes,
               1 == bytes ? " " : "s ",
               alignment,
               address);

        std::fflush(stdout);
    }

    return address;
}

void test_resource::do_deallocate(void *p, size_t bytes, size_t alignment)
{
    lock_guard guard{ m_lock_ };

    m_deallocations_.fetch_add(1, memory_order_relaxed);
    m_last_deallocated_address_.store(p, memory_order_relaxed);

    if (nullptr == p) {
        if (0 != bytes) {
            m_bad_deallocate_params_.fetch_add(1, memory_order_relaxed);
            if (!is_quiet()) {
                formatBadBytesForNullptr(bytes, alignment);
                if (!is_no_abort()) {
                    std::abort();                                      // ABORT
                }
            }
        }
        else {
            m_last_deallocated_num_bytes_.store(0,
                                                memory_order_relaxed);
            m_last_deallocated_alignment_.store(alignment,
                                                memory_order_relaxed);
        }
        return;                                                       // RETURN
    }

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

    if (allocatedMemoryPattern != head->m_object_.m_magic_number_) {
        miscError = true;
    }
    else if (this != head->m_object_.m_pmr_) {
        miscError = true;
    }
    else {
        size            = head->m_object_.m_bytes_;
        allocationIndex = head->m_object_.m_index_;
    }

    // If there is evidence of corruption, this memory may have already been
    // freed.  On some platforms (but not others), the 'free' function will
    // scribble freed memory.  To get uniform behavior for test drivers, we
    // deliberately don't check over/underruns if 'miscError' is 'true'.

    int overrunBy  = 0;
    int underrunBy = 0;

    if (!miscError) {
        byte *pcBegin;
        byte *pcEnd;

        // Check the padding before the segment.  Go backwards so we will
        // report the trashed byte nearest the segment.

        pcBegin = (byte *)p - 1;
        pcEnd   = (byte *)&head->m_object_.m_padding_;

        for (byte *pc = pcBegin; pcEnd <= pc; --pc) {
            if (paddedMemoryByte != *pc) {
                underrunBy = static_cast<int>(pcBegin + 1 - pc);
                break;
            }
        }

        if (!underrunBy) {
            // Check the padding after the segment.

            byte *tail = (byte *)p + size;
            pcBegin = tail;
            pcEnd = tail + paddingSize;
            for (byte *pc = pcBegin; pc < pcEnd; ++pc) {
                if (paddedMemoryByte != *pc) {
                    overrunBy = static_cast<int>(pc + 1 - pcBegin);
                    break;
                }
            }
        }

        if (bytes != size || alignment != head->m_object_.m_alignment_) {
            paramError = true;
        }
    }

    // Now check for corrupted memory block and cross allocation.

    if (!miscError && !overrunBy && !underrunBy &&!paramError) {
        m_pmr_->deallocate(removeLink(m_list_,
                                      head->m_object_.m_address_),
                           sizeof(Link), alignof(Link));
    }
    else { // Any error, count it, report it
        if (miscError) {
            m_mismatches_.fetch_add(1, memory_order_relaxed);
        }
        if (paramError) {
            m_bad_deallocate_params_.fetch_add(1, memory_order_relaxed);
        }
        if (overrunBy || underrunBy) {
            m_bounds_errors_.fetch_add(1, memory_order_relaxed);
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

    m_last_deallocated_num_bytes_.store(static_cast<long long>(size),
                                        memory_order_relaxed);
    m_last_deallocated_alignment_.store(static_cast<long long>(alignment),
                                        memory_order_relaxed);

    m_blocks_in_use_.fetch_add(-1, memory_order_relaxed);

    m_bytes_in_use_.fetch_add(-static_cast<long long>(size),
                              memory_order_relaxed);

    head->m_object_.m_magic_number_ = deallocatedMemoryPattern;

    std::memset(p, static_cast<int>(scribbledMemoryByte), size);

    if (is_verbose()) {

        // In verbose mode, print a message to 'stdout' -- e.g.,
        //..
        //  TestAllocator local [245]: Deallocated 1 byte at 0x3c1b2740.
        //..

        printf("test_resource");

        if (!m_name_.empty()) {
            printf(" %.*s",
                   static_cast<int>(m_name_.length()), m_name_.data());
        }

        printf(" [%lld]: Deallocated %zu byte%s(aligned %zu) at %p.\n",
               allocationIndex,
               size,
               1 == size ? " " : "s ",
               alignment,
               p);

        std::fflush(stdout);
    }

    m_pmr_->deallocate(head, sizeof(AlignedHeader) + size + paddingSize);
}

bool test_resource::do_is_equal(const memory_resource& that) const noexcept
{
    return this == &that;
}

void test_resource::print() const noexcept
{
    lock_guard guard{ m_lock_ };

    if (!m_name_.empty()) {
        printf("\n"
               "==================================================\n"
               "                TEST RESOURCE %.*s STATE\n"
               "--------------------------------------------------\n",
               static_cast<int>(m_name_.length()), m_name_.data());
    }
    else {
        printf("\n"
               "==================================================\n"
               "                TEST RESOURCE STATE\n"
               "--------------------------------------------------\n");
    }

    printf("        Category\tBlocks\tBytes\n"
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

    if (m_list_->d_head_p) {
        printf(" Indices of Outstanding Memory Allocations:\n ");
        printList(*m_list_);
    }
    std::fflush(stdout);
}

long long test_resource::status() const noexcept
{
    static const int memoryLeak = -1;
    static const int success = 0;

    lock_guard guard{ m_lock_ };

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
