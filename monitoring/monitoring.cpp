#include <supportlib/framer.h>
#include <supportlib/assert.h>

#include <memory_resource>

#include <vector>
#include <string>

void test()
{
    Framer framer{ "Monitoring" };

    std::pmr::test_resource dr{ "default" };
    std::pmr::test_resource_monitor drm{ dr };
    std::pmr::default_resource_guard drg{ &dr };

    std::pmr::test_resource tr{ "object" };

    const char *longstr = "A very very long string that allocates memory";

    std::pmr::vector<std::pmr::string> vec{ &tr };
    vec.emplace_back(longstr);
    vec.emplace_back(longstr);

    ASSERT(drm.is_in_use_same());
    ASSERT(drm.is_total_same());
}

int main()
{
    test();
}

// ----------------------------------------------------------------------------
// Copyright 2018 Bloomberg Finance L.P.
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
