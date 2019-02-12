// MIT License
//
// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Google Test
#include <gtest/gtest.h>

// Thrust
#include <thrust/tabulate.h>
#include <thrust/functional.h>
#include <thrust/iterator/discard_iterator.h>
#include <thrust/iterator/retag.h>
#include <thrust/device_vector.h>

// HIP API
#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC
#include <hip/hip_runtime_api.h>
#include <hip/hip_runtime.h>

#define HIP_CHECK(condition) ASSERT_EQ(condition, hipSuccess)
#endif // THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC

#include "test_utils.hpp"

template<
    class InputType
>
struct Params
{
    using input_type = InputType;
};

template<class Params>
class FindTests : public ::testing::Test
{
public:
    using input_type = typename Params::input_type;
};

typedef ::testing::Types<
    Params<thrust::host_vector<short>>,
    Params<thrust::host_vector<int>>,
    Params<thrust::host_vector<long long>>,
    Params<thrust::host_vector<unsigned short>>,
    Params<thrust::host_vector<unsigned int>>,
    Params<thrust::host_vector<unsigned long long>>,
    Params<thrust::host_vector<float>>,
    Params<thrust::host_vector<double>>,
    Params<thrust::device_vector<short>>,
    Params<thrust::device_vector<int>>,
    Params<thrust::device_vector<long long>>,
    Params<thrust::device_vector<unsigned short>>,
    Params<thrust::device_vector<unsigned int>>,
    Params<thrust::device_vector<unsigned long long>>,
    Params<thrust::device_vector<float>>,
    Params<thrust::device_vector<double>>
> FindTestsParams;

TYPED_TEST_CASE(FindTests, FindTestsParams);

#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC

template <typename T>
struct equal_to_value_pred
{
    T value;

    equal_to_value_pred(T value) : value(value) {}

    __host__ __device__
    bool operator()(T v) const { return v == value; }
};

template <typename T>
struct not_equal_to_value_pred
{
    T value;

    not_equal_to_value_pred(T value) : value(value) {}

    __host__ __device__
    bool operator()(T v) const { return v != value; }
};

template<typename T>
struct less_than_value_pred
{
    T value;

    less_than_value_pred(T value) : value(value) {}

    __host__ __device__
    bool operator()(T v) const { return v < value; }
};

TYPED_TEST(FindTests, TestFindSimple)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  Vector vec(5);
  vec[0] = T(1);
  vec[1] = T(2);
  vec[2] = T(3);
  vec[3] = T(3);
  vec[4] = T(5);

  ASSERT_EQ(thrust::find(vec.begin(), vec.end(), T(0)) - vec.begin(), 5);
  ASSERT_EQ(thrust::find(vec.begin(), vec.end(), T(1)) - vec.begin(), 0);
  ASSERT_EQ(thrust::find(vec.begin(), vec.end(), T(2)) - vec.begin(), 1);
  ASSERT_EQ(thrust::find(vec.begin(), vec.end(), T(3)) - vec.begin(), 2);
  ASSERT_EQ(thrust::find(vec.begin(), vec.end(), T(4)) - vec.begin(), 5);
  ASSERT_EQ(thrust::find(vec.begin(), vec.end(), T(5)) - vec.begin(), 4);
}

template<typename InputIterator, typename T>
InputIterator find(my_system &system, InputIterator first, InputIterator, const T&)
{
    system.validate_dispatch();
    return first;
}

TEST(FindTests, TestFindDispatchExplicit)
{
  thrust::device_vector<int> vec(1);

  my_system sys(0);
  thrust::find(sys,
               vec.begin(),
               vec.end(),
               0);

  ASSERT_EQ(true, sys.is_valid());
}

template<typename InputIterator, typename T>
InputIterator find(my_tag, InputIterator first, InputIterator, const T&)
{
    *first = 13;
    return first;
}

TEST(FindTests, TestFindDispatchImplicit)
{
  thrust::device_vector<int> vec(1);

  thrust::find(thrust::retag<my_tag>(vec.begin()),
               thrust::retag<my_tag>(vec.end()),
               0);

  ASSERT_EQ(13, vec.front());
}

TYPED_TEST(FindTests, TestFindIfSimple)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  Vector vec(5);
  vec[0] = T(1);
  vec[1] = T(2);
  vec[2] = T(3);
  vec[3] = T(3);
  vec[4] = T(5);

  ASSERT_EQ(thrust::find_if(vec.begin(), vec.end(), equal_to_value_pred<T>(0)) - vec.begin(), 5);
  ASSERT_EQ(thrust::find_if(vec.begin(), vec.end(), equal_to_value_pred<T>(1)) - vec.begin(), 0);
  ASSERT_EQ(thrust::find_if(vec.begin(), vec.end(), equal_to_value_pred<T>(2)) - vec.begin(), 1);
  ASSERT_EQ(thrust::find_if(vec.begin(), vec.end(), equal_to_value_pred<T>(3)) - vec.begin(), 2);
  ASSERT_EQ(thrust::find_if(vec.begin(), vec.end(), equal_to_value_pred<T>(4)) - vec.begin(), 5);
  ASSERT_EQ(thrust::find_if(vec.begin(), vec.end(), equal_to_value_pred<T>(5)) - vec.begin(), 4);
}

template<typename InputIterator, typename Predicate>
InputIterator find_if(my_system &system, InputIterator first, InputIterator, Predicate)
{
    system.validate_dispatch();
    return first;
}

TEST(FindTests, TestFindIfDispatchExplicit)
{
  thrust::device_vector<int> vec(1);

  my_system sys(0);
  thrust::find_if(sys,
                  vec.begin(),
                  vec.end(),
                  thrust::identity<int>());

  ASSERT_EQ(true, sys.is_valid());
}

template<typename InputIterator, typename Predicate>
InputIterator find_if(my_tag, InputIterator first, InputIterator, Predicate)
{
    *first = 13;
    return first;
}


TEST(FindTests, TestFindIfDispatchImplicit)
{
  thrust::device_vector<int> vec(1);

  thrust::find_if(thrust::retag<my_tag>(vec.begin()),
                  thrust::retag<my_tag>(vec.end()),
                  thrust::identity<int>());

  ASSERT_EQ(13, vec.front());
}

TYPED_TEST(FindTests, TestFindIfNotSimple)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  Vector vec(5);
  vec[0] = T(0);
  vec[1] = T(1);
  vec[2] = T(2);
  vec[3] = T(3);
  vec[4] = T(4);

  ASSERT_EQ(0, thrust::find_if_not(vec.begin(), vec.end(), less_than_value_pred<T>(0)) - vec.begin());
  ASSERT_EQ(1, thrust::find_if_not(vec.begin(), vec.end(), less_than_value_pred<T>(1)) - vec.begin());
  ASSERT_EQ(2, thrust::find_if_not(vec.begin(), vec.end(), less_than_value_pred<T>(2)) - vec.begin());
  ASSERT_EQ(3, thrust::find_if_not(vec.begin(), vec.end(), less_than_value_pred<T>(3)) - vec.begin());
  ASSERT_EQ(4, thrust::find_if_not(vec.begin(), vec.end(), less_than_value_pred<T>(4)) - vec.begin());
  ASSERT_EQ(5, thrust::find_if_not(vec.begin(), vec.end(), less_than_value_pred<T>(5)) - vec.begin());
}

template<typename InputIterator, typename Predicate>
InputIterator find_if_not(my_system &system, InputIterator first, InputIterator, Predicate)
{
    system.validate_dispatch();
    return first;
}

TEST(FindTests, TestFindIfNotDispatchExplicit)
{
  thrust::device_vector<int> vec(1);

  my_system sys(0);
  thrust::find_if_not(sys,
                      vec.begin(),
                      vec.end(),
                      thrust::identity<int>());

  ASSERT_EQ(true, sys.is_valid());
}

template<typename InputIterator, typename Predicate>
InputIterator find_if_not(my_tag, InputIterator first, InputIterator, Predicate)
{
    *first = 13;
    return first;
}

TEST(FindTests, TestFindIfNotDispatchImplicit)
{
  thrust::device_vector<int> vec(1);

  thrust::find_if_not(thrust::retag<my_tag>(vec.begin()),
                      thrust::retag<my_tag>(vec.end()),
                      thrust::identity<int>());

  ASSERT_EQ(13, vec.front());
}

TYPED_TEST(FindTests, TestFind)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  using HostIterator = typename thrust::host_vector<T>::iterator;
  using DeviceIterator = typename thrust::device_vector<T>::iterator;

  const std::vector<size_t> sizes = get_sizes();
  for(auto size : sizes)
  {
    thrust::host_vector<T> h_data = get_random_data<T>(size,
                                                       std::numeric_limits<T>::min(),
                                                       std::numeric_limits<T>::max());
    thrust::device_vector<T> d_data = h_data;

    HostIterator h_iter;
    DeviceIterator d_iter;

    h_iter = thrust::find(h_data.begin(), h_data.end(), T(0));
    d_iter = thrust::find(d_data.begin(), d_data.end(), T(0));
    ASSERT_EQ(h_iter - h_data.begin(), d_iter - d_data.begin());

    for (size_t i = 1; i < size; i *= 2)
    {
        T sample = h_data[i];
        h_iter = thrust::find(h_data.begin(), h_data.end(), sample);
        d_iter = thrust::find(d_data.begin(), d_data.end(), sample);
        ASSERT_EQ(h_iter - h_data.begin(), d_iter - d_data.begin());
    }
  }
}

TYPED_TEST(FindTests, TestFindIf)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  using HostIterator = typename thrust::host_vector<T>::iterator;
  using DeviceIterator = typename thrust::device_vector<T>::iterator;

  const std::vector<size_t> sizes = get_sizes();
  for(auto size : sizes)
  {
    thrust::host_vector<T> h_data = get_random_data<T>(size,
                                                       std::numeric_limits<T>::min(),
                                                       std::numeric_limits<T>::max());
    thrust::device_vector<T> d_data = h_data;

    HostIterator h_iter;
    DeviceIterator d_iter;

    h_iter = thrust::find_if(h_data.begin(), h_data.end(), equal_to_value_pred<T>(0));
    d_iter = thrust::find_if(d_data.begin(), d_data.end(), equal_to_value_pred<T>(0));
    ASSERT_EQ(h_iter - h_data.begin(), d_iter - d_data.begin());

    for (size_t i = 1; i < size; i *= 2)
    {
        T sample = h_data[i];
        h_iter = thrust::find_if(h_data.begin(), h_data.end(), equal_to_value_pred<T>(sample));
        d_iter = thrust::find_if(d_data.begin(), d_data.end(), equal_to_value_pred<T>(sample));
        ASSERT_EQ(h_iter - h_data.begin(), d_iter - d_data.begin());
    }
  }
}

TYPED_TEST(FindTests, TestFindIfNot)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  using HostIterator = typename thrust::host_vector<T>::iterator;
  using DeviceIterator = typename thrust::device_vector<T>::iterator;

  const std::vector<size_t> sizes = get_sizes();
  for(auto size : sizes)
  {
    thrust::host_vector<T> h_data = get_random_data<T>(size,
                                                       std::numeric_limits<T>::min(),
                                                       std::numeric_limits<T>::max());
    thrust::device_vector<T> d_data = h_data;

    HostIterator h_iter;
    DeviceIterator d_iter;

    h_iter = thrust::find_if_not(h_data.begin(), h_data.end(), not_equal_to_value_pred<T>(0));
    d_iter = thrust::find_if_not(d_data.begin(), d_data.end(), not_equal_to_value_pred<T>(0));
    ASSERT_EQ(h_iter - h_data.begin(), d_iter - d_data.begin());

    for (size_t i = 1; i < size; i *= 2)
    {
        T sample = h_data[i];
        h_iter = thrust::find_if_not(h_data.begin(), h_data.end(), not_equal_to_value_pred<T>(sample));
        d_iter = thrust::find_if_not(d_data.begin(), d_data.end(), not_equal_to_value_pred<T>(sample));
        ASSERT_EQ(h_iter - h_data.begin(), d_iter - d_data.begin());
    }
  }
}

#endif // THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC