// assert.h                                                           -*-C++-*-
#ifndef SUPPORTLIB_ASSERT_H_INCLUDED
#define SUPPORTLIB_ASSERT_H_INCLUDED

#include <iostream>

#ifndef SUPPORTLIB_ASSERT_REGISTER_ERROR
#define SUPPORTLIB_ASSERT_REGISTER_ERROR
#endif

#define ASSERT(a)                                                             \
    if (!a) {                                                                 \
        std::cout << #a << " != true\n";                                      \
        SUPPORTLIB_ASSERT_REGISTER_ERROR                                      \
    }

#define ASSERT_EQ(a, b)                                                       \
    if (a != b) {                                                             \
        std::cout << #a << " != " << #b << '\n';                              \
        std::cout << a << " != " << b << '\n';                                \
        SUPPORTLIB_ASSERT_REGISTER_ERROR                                      \
    }

#endif

// ----------------------------------------------------------------------------
// Copyright 2019 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License") with the LLVM
// exception (the "Exception"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License and the Exception at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//     
//     https://spdx.org/licenses/LLVM-exception.html
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
