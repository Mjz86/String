/*MIT License

Copyright (c) 2025 Mjz86

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "../optional_ref.hpp"
#ifndef MJZ_THREADS_BASE_LIB_HPP_FILE_
#define MJZ_THREADS_BASE_LIB_HPP_FILE_
MJZ_EXPORT namespace mjz::threads_ns {
  template <class lambda_t, class... args_t>
  concept mjz_jthread_can_create_c_ = requires(lambda_t lambda) {
    requires callable_anyret_c<std::decay_t<lambda_t>,
                               void(std::decay_t<args_t>...) noexcept>;
    { std::decay_t<lambda_t>(std::forward<lambda_t>(lambda)) } noexcept;

    requires((requires(args_t args) {
               {
                 std::forward<args_t>(args)
               } noexcept -> std::convertible_to<std::decay_t<args_t>>;
               { std::decay_t<args_t>(std::forward<args_t>(args)) } noexcept;
             }) && ...);
  };
  MJZ_FCONSTANT(uint64_t) defult_timeout = uint64_t(uint8_t(-1));
}  // namespace mjz::threads_ns
#endif  // MJZ_THREADS_BASE_LIB_HPP_FILE_