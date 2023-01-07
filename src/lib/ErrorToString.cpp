#include "ergovk.hpp"
#include <unordered_map>

namespace {
    const static std::unordered_map<ergovk::InitializeError, std::string_view> initialize_errors=
    {
        {ergovk::InitializeError::AllocatorCreate, "Unable to create memory allocator"},
        {ergovk::InitializeError::FailedCreate, "Unable to create instance"},
        {ergovk::InitializeError::NoSuitableGpu, "No suitable GPU found"},
        {ergovk::InitializeError::SurfaceCreate, "Unable to acquire surface"}
    };
}

namespace ergovk
{
    std::string_view error_to_string(const InitializeError& error)
    {
        auto it = initialize_errors.find(error);
        if (it != initialize_errors.end())
        {
            return it->second;
        }
        assert(false);
        return "";
    }

}