#include "../optional_ref.hpp"
#ifndef MJZ_THREADS_BASE_LIB_HPP_FILE_
#define MJZ_THREADS_BASE_LIB_HPP_FILE_
namespace mjz::threads_ns {
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
MJZ_CONSTANT(uint64_t) defult_timeout = uint64_t(uint8_t(-1));
}  // namespace mjz::threads_ns
#endif  // MJZ_THREADS_BASE_LIB_HPP_FILE_