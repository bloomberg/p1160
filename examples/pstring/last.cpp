// psrting_last.cpp                                                   -*-C++-*-
#include <pstring_last.h>

#include <supportlib/framer.h>

#define SUPPORTLIB_ASSERT_REGISTER_ERROR ++errorCount;
#include <supportlib/assert.h>

#include <test_resource.hpp>

#include <utility>

void breathing_test  (bool verbose);
void copy_test       (bool verbose);
void assign_test     (bool verbose);
void self_assign_test(bool verbose);
void move_test       (bool verbose);

int errorCount{ 0 };

void tests(bool verbose)
{
    breathing_test  (verbose);
    copy_test       (verbose);
    assign_test     (verbose);
    self_assign_test(verbose);
    move_test       (verbose);
}

int main()
{
    tests(false);

    tests(true);

    return errorCount;
}

void breathing_test(bool verbose)
{
    Framer framer{ "breathing test", verbose };

    std::pmr::test_resource tpmr{ "object", verbose };

    pstring astring{ "barfool", &tpmr };

    ASSERT_EQ(astring.str(), "barfool");
}

void copy_test(bool verbose)
{
    Framer framer{ "copy constructor test", verbose };

    std::pmr::test_resource tpmr{ "object", verbose };
    tpmr.set_no_abort(true);

    pstring astring{ "barfool", &tpmr };
    pstring string2{ astring, &tpmr };
    pstring string3{ astring };

    ASSERT_EQ(astring.str(), "barfool");
    ASSERT_EQ(string2.str(), "barfool");
    ASSERT_EQ(string3.str(), "barfool");
}

void assign_test(bool verbose)
{
    Framer framer{ "copy assignment test", verbose };

    std::pmr::test_resource tpmr{ "stage7", verbose };
    tpmr.set_no_abort(true);

    pstring astring{ "barfool", &tpmr };
    pstring string2{ "abc12", &tpmr };

    string2 = astring;

    ASSERT_EQ(astring.str(), "barfool");
    ASSERT_EQ(string2.str(), "barfool");
}

void self_assign_test(bool verbose)
{
    Framer framer{ "self assignment test", verbose };

    std::pmr::test_resource tpmr{ "stage7", verbose };
    tpmr.set_no_abort(true);

    pstring astring{ "barfool", &tpmr };

    astring = astring;

    ASSERT_EQ(astring.str(), "barfool");
}

void move_test(bool verbose)
{
    Framer framer{ "move constructor", verbose };

    std::pmr::test_resource           dr{ "default" };
    std::pmr::test_resource_monitor  drm{ dr };
    std::pmr::default_resource_guard drg{ &dr };
    dr.set_verbose(verbose);

    std::pmr::test_resource          tr{ "object" };
    std::pmr::test_resource_monitor trm{ tr };
    tr.set_verbose(verbose);

    pstring astring{ "barfool", &tr };
    ASSERT(trm.is_total_up());
    ASSERT_EQ(trm.delta_blocks_in_use(), 1);
    trm.reset();

    pstring mstring{ std::move(astring) };

    ASSERT(drm.is_in_use_same()); // Did not allocate`bstring` with this
    ASSERT(drm.is_total_same());  // Did not allocate /anything/ with this
}

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
