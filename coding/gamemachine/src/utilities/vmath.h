#ifndef __VMATH_H__
#define __VMATH_H__

#define VEC3(v4) vmath::vec3(v4[0], v4[1], v4[2])
#define VEC4(v3, v4) vmath::vec4(v3[0], v3[1], v3[2], v4[3])

#define _USE_MATH_DEFINES  1 // Include constants defined in math.h
#include <math.h>

namespace vmath
{
	inline float fastInvSqrt(float x)
	{
		float xhalf = 0.5f*x;
		int i = *(int*)&x; // get bits for floating VALUE
		i = 0x5f375a86 - (i >> 1); // gives initial guess y0
		x = *(float*)&i; // convert bits BACK to float
		x = x*(1.5f - xhalf*x*x); // Newton step, repeating increases accuracy
		return x;
	}

	template <typename T, const int w, const int h> class matNM;
	template <typename T, const int len> class vecN;
	template <typename T> class Tquaternion;

	template <typename T>
	inline T degrees(T angleInRadians)
	{
		return angleInRadians * static_cast<T>(180.0 / M_PI);
	}

	template <typename T>
	inline T radians(T angleInDegrees)
	{
		return angleInDegrees * static_cast<T>(M_PI / 180.0);
	}

	template <typename T>
	struct random
	{
		operator T ()
		{
			static unsigned int seed = 0x13371337;
			unsigned int res;
			unsigned int tmp;

			seed *= 16807;

			tmp = seed ^ (seed >> 4) ^ (seed << 15);

			res = (tmp >> 9) | 0x3F800000;

			return static_cast<T>(res);
		}
	};

	template<>
	struct random<float>
	{
		operator float()
		{
			static unsigned int seed = 0x13371337;
			float res;
			unsigned int tmp;

			seed *= 16807;

			tmp = seed ^ (seed >> 4) ^ (seed << 15);

			*((unsigned int *)&res) = (tmp >> 9) | 0x3F800000;

			return (res - 1.0f);
		}
	};

	template<>
	struct random<unsigned int>
	{
		operator unsigned int()
		{
			static unsigned int seed = 0x13371337;
			unsigned int res;
			unsigned int tmp;

			seed *= 16807;

			tmp = seed ^ (seed >> 4) ^ (seed << 15);

			res = (tmp >> 9) | 0x3F800000;

			return res;
		}
	};

#if USE_SIMD
	template<int len>
	inline static void simd_float(const float* vec, float* out, bool* support)
	{
		if (len != 2 && len != 3 && len != 4)
		{
			*support = false;
			return;
		}

		for (int i = 0; i < 4; i++)
		{
			if (i < len)
				out[i] = vec[i];
			else
				out[i] = 0;
		}
		*support = true;
	}

#endif

	// vector div x
	template <typename vec_type, typename T>
	static inline vec_type div(const vec_type& _left, T _right)
	{
#if USE_SIMD
		bool support;
		GM_SIMD_float f_left[4], f_right[4];
		simd_float<vec_type::dimension>(&_left[0], f_left, &support);
		if (!support)
		{
			return regular<vec_type::dimension, T>::div(_right, _left);
		}

		T data_arr[] = { _right, _right, _right, _right };
		simd_float<vec_type::dimension>(data_arr, f_right, &support);
		GM_SIMD_float f_result[4];
		__m128 __left = _mm_load_ps(&f_left[0]);
		__m128 __right = _mm_load_ps(&f_right[0]);
		__m128 __result = _mm_div_ps(__left, __right);
		_mm_store_ps(f_result, __result);

		vec_type result;
		memcpy(&result[0], f_result, sizeof(f_result[0]) * vec_type::dimension);
		return result;
#else
		return regular<vec_type::dimension, T>::div(_left, _right);
#endif
	}

	// x div vector
	template <typename vec_type, typename T>
	static inline vec_type div(T _left, const vec_type& _right)
	{
#if USE_SIMD
		bool support;
		GM_SIMD_float f_left[4], f_right[4];
		simd_float<vec_type::dimension>(&_right[0], f_right, &support);
		if (!support)
		{
			return regular<vec_type::dimension, T>::div(_left, _right);
		}

		T left_arr[] = { _left, _left, _left, _left };
		simd_float<vec_type::dimension>(left_arr, f_left, &support);
		GM_SIMD_float f_result[4];
		__m128 __this = _mm_load_ps(&f_left[0]);
		__m128 __that = _mm_load_ps(&f_right[0]);
		__m128 __result = _mm_div_ps(__this, __that);
		_mm_store_ps(f_result, __result);

		vec_type result;
		memcpy(&result[0], f_result, sizeof(f_result[0]) * vec_type::dimension);
		return result;
#else
		return regular<vec_type::dimension, T>::div(_left, _right);
#endif
	}

	template <typename vec_type>
	static inline vec_type div(const vec_type& _left, const vec_type& _right)
	{
#if USE_SIMD
		bool support;
		GM_SIMD_float f_left[4], f_right[4];
		simd_float<vec_type::dimension>(&_left[0], f_left, &support);
		if (!support)
		{
			return regular<vec_type::dimension, T>::div(_left, _right);
		}

		simd_float<vec_type::dimension>(&_right[0], f_right, &support);
		GM_SIMD_float f_result[4];
		__m128 __left = _mm_load_ps(&f_left[0]);
		__m128 __right = _mm_load_ps(&f_right[0]);
		__m128 __result = _mm_div_ps(__left, __right);
		_mm_store_ps(f_result, __result);

		vec_type result;
		memcpy(&result[0], f_result, sizeof(f_result[0]) * vec_type::dimension);
		return result;
#else
		return regular<vec_type::dimension, T>::div(_left, _right);
#endif
	}

	template <typename vec_type>
	static inline const vec_type add(const vec_type& _left, const vec_type& _right)
	{
		vec_type result;
#if USE_SIMD
		bool support;
		GM_SIMD_float f_left[4], f_right[4];
		simd_float<vec_type::dimension>(&_left[0], f_left, &support);
		if (!support)
		{
			for (int n = 0; n < vec_type::dimension; n++)
				result[n] = _left[n] + _right[n];
			return result;
		}
		simd_float<vec_type::dimension>(&_right[0], f_right, &support);
		GM_SIMD_float f_result[4];
		__m128 __left = _mm_load_ps(&f_left[0]);
		__m128 __right = _mm_load_ps(&f_right[0]);
		__m128 __result = _mm_add_ps(__left, __right);
		_mm_store_ps(f_result, __result);
		memcpy(&result[0], f_result, sizeof(f_result[0]) * vec_type::dimension);
#else
		for (int n = 0; n < vec_type::dimension; n++)
			result[n] = _left[n] + _right[n];
#endif
		return result;
	}

	template <typename vec_type>
	static inline const vec_type subtract(const vec_type& _left, const vec_type& _right)
	{
		vec_type result;
#if USE_SIMD
		bool support;
		GM_SIMD_float f_left[4], f_right[4];
		simd_float<vec_type::dimension>(&_left[0], f_left, &support);
		if (!support)
		{
			for (int n = 0; n < vec_type::dimension; n++)
				result[n] = _left[n] + _right[n];
			return result;
		}
		simd_float<vec_type::dimension>(&_right[0], f_right, &support);
		GM_SIMD_float f_result[4];
		__m128 __left = _mm_load_ps(&f_left[0]);
		__m128 __right = _mm_load_ps(&f_right[0]);
		__m128 __result = _mm_sub_ps(__left, __right);
		_mm_store_ps(f_result, __result);
		memcpy(&result[0], f_result, sizeof(f_result[0]) * vec_type::dimension);
#else
		for (int n = 0; n < vec_type::dimension; n++)
			result[n] = _left[n] + _right[n];
#endif
		return result;
	}

	template <typename vec_type, typename T>
	static inline vec_type mul(T _left, const vec_type& _right)
	{
		vec_type result;
#if USE_SIMD
		bool support;
		GM_SIMD_float f_left[4], f_right[4];
		T left_arr[] = { _left, _left, _left, _left };
		simd_float<vec_type::dimension>(left_arr, f_left, &support);
		if (!support)
		{
			for (int n = 0; n < vec_type::dimension; n++)
				result[n] = _right[n] * _left;
			return result;
		}

		simd_float<vec_type::dimension>(&_right[0], f_right, &support);
		GM_SIMD_float f_result[4];
		__m128 __left = _mm_load_ps(&f_left[0]);
		__m128 __right = _mm_load_ps(&f_right[0]);
		__m128 __result = _mm_mul_ps(__right, __left);
		_mm_store_ps(f_result, __result);
		memcpy(&result[0], f_result, sizeof(f_result[0]) * vec_type::dimension);
#else
		for (int n = 0; n < vec_type::dimension; n++)
			result[n] = _right[n] * _left;
#endif
		return result;
	}

	template <typename vec_type>
	static inline vec_type mul(vec_type _left, const vec_type& _right)
	{
		vec_type result;
#if USE_SIMD
		bool support;
		GM_SIMD_float f_left[4], f_right[4];
		simd_float<vec_type::dimension>(&_left[0], f_left, &support);
		if (!support)
		{
			for (int n = 0; n < vec_type::dimension; n++)
				result[n] = _right[n] * _left;
			return result;
		}

		simd_float<vec_type::dimension>(&_right[0], f_right, &support);
		GM_SIMD_float f_result[4];
		__m128 __left = _mm_load_ps(&f_left[0]);
		__m128 __right = _mm_load_ps(&f_right[0]);
		__m128 __result = _mm_mul_ps(__right, __left);
		_mm_store_ps(f_result, __result);
		memcpy(&result[0], f_result, sizeof(f_result[0]) * vec_type::dimension);
#else
		for (int n = 0; n < vec_type::dimension; n++)
			result[n] = _right[n] * _left;
#endif
		return result;
	}

	template <typename vec_type, typename T>
	static inline T fast_dot(const vec_type& _left, const vec_type& _right)
	{
		T result = T(0);
#if USE_SIMD
		bool support;
		GM_SIMD_float f_left[4], f_right[4], f_result[4];
		simd_float<vec_type::dimension>(&_left[0], f_left, &support);
		if (!support)
		{
			for (int n = 0; n < vec_type::dimension; n++)
			{
				result += _left[n] * _right[n];
			}
		}
		simd_float<vec_type::dimension>(&_right[0], f_right, &support);
		__m128 __left = _mm_load_ps(&f_left[0]);
		__m128 __right = _mm_load_ps(&f_right[0]);
		__m128 __result = _mm_mul_ps(__left, __right);
		_mm_store_ps(f_result, __result);
		for (int i = 0; i < vec_type::dimension; i++)
		{
			result += f_result[i];
		}
#else
		for (int n = 0; n < vec_type::dimension; n++)
		{
			result += _left[n] * _right[n];
		}
#endif
		return result;
	}

	template <typename T>
	static inline vecN<T, 3> fast_cross(const vecN<T, 3>& _left, const vecN<T, 3>& _right)
	{
#if USE_SIMD
		GM_SIMD_float	result[4];
		GM_SIMD_float	t0[] = { _left[1],  _left[2],  _left[0],  0 },
						t1[] = { _right[2], _right[0], _right[1], 0 },
						t2[] = { _left[2],  _left[0],  _left[1],  0 },
						t3[] = { _right[1], _right[2], _right[0], 0 };
		__m128			__t0 = _mm_load_ps(t0),
						__t1 = _mm_load_ps(t1),
						__t2 = _mm_load_ps(t2),
						__t3 = _mm_load_ps(t3);
		__m128			__m1 = _mm_mul_ps(__t0, __t1),
						__m2 = _mm_mul_ps(__t2, __t3);
		__m128			__r  = _mm_sub_ps(__m1, __m2);
		_mm_store_ps(result, __r);
		return Tvec3<T>(result[0], result[1], result[2]);
#else
		return Tvec3<T>(a[1] * b[2] - b[1] * a[2],
			a[2] * b[0] - b[2] * a[0],
			a[0] * b[1] - b[0] * a[1]);
#endif
	}

	// Vector
	template <typename T, const int len>
	class vecN
	{
	public:
		typedef class vecN<T, len> my_type;
		typedef T element_type;
		enum { dimension = len };

		// Default constructor does nothing, just like built-in types
		inline vecN()
		{
			// Uninitialized variable
		}

		// Copy constructor
		inline vecN(const vecN& that)
		{
			assign(that);
		}

		// Construction from scalar
		inline vecN(T s)
		{
			int n;
			for (n = 0; n < len; n++)
			{
				data[n] = s;
			}
		}

		// Assignment operator
		inline vecN& operator=(const vecN& that)
		{
			assign(that);
			return *this;
		}

		inline vecN& operator=(const T& that)
		{
			int n;
			for (n = 0; n < len; n++)
				data[n] = that;

			return *this;
		}

		inline vecN operator+(const vecN& that) const
		{
			return add<vecN>(*this, that);
		}

		inline vecN& operator+=(const vecN& that)
		{
			return (*this = *this + that);
		}

		inline vecN operator-() const
		{
			my_type result;
			int n;
			for (n = 0; n < len; n++)
				result.data[n] = -data[n];
			return result;
		}

		inline vecN operator-(const vecN& that) const
		{
			return subtract<vecN>(*this, that);
		}

		inline vecN& operator-=(const vecN& that)
		{
			return (*this = *this - that);
		}

		inline vecN operator*(const vecN& that) const
		{
			return mul(that, *this);
		}

		inline vecN& operator*=(const vecN& that)
		{
			return (*this = *this * that);
		}

		inline vecN operator*(const T& that) const
		{
			return mul(that, *this);
		}

		inline vecN& operator*=(const T& that)
		{
			assign(*this * that);

			return *this;
		}

		inline vecN operator/(const vecN& that) const
		{
			return div<vecN, T>(*this, that);
		}

		inline vecN& operator/=(const vecN& that)
		{
			assign(*this / that);

			return *this;
		}

		inline vecN operator/(const T& that) const
		{
			return div<vecN, T>(*this, that);
		}

		inline vecN& operator/=(const T& that)
		{
			assign(*this / that);
			return *this;
		}

		inline T& operator[](int n) { return data[n]; }
		inline const T& operator[](int n) const { return data[n]; }

		inline static int size(void) { return len; }

		inline operator const T* () const { return &data[0]; }

		static inline vecN random()
		{
			vecN result;
			int i;

			for (i = 0; i < len; i++)
			{
				result[i] = vmath::random<T>();
			}
			return result;
		}

	protected:
		T data[len];

		inline void assign(const vecN& that)
		{
			int n;
			for (n = 0; n < len; n++)
				data[n] = that.data[n];
		}
	};

	template <typename T>
	class Tvec2 : public vecN<T, 2>
	{
	public:
		typedef vecN<T, 2> base;

		// Uninitialized variable
		inline Tvec2() {}
		// Copy constructor
		inline Tvec2(const base& v) : base(v) {}

		// vec2(x, y);
		inline Tvec2(T x, T y)
		{
			base::data[0] = x;
			base::data[1] = y;
		}
	};

	template <typename T>
	class Tvec3 : public vecN<T, 3>
	{
	public:
		typedef vecN<T, 3> base;

		// Uninitialized variable
		inline Tvec3() {}

		// Copy constructor
		inline Tvec3(const base& v) : base(v) {}

		// vec3(x, y, z);
		inline Tvec3(T x, T y, T z)
		{
			base::data[0] = x;
			base::data[1] = y;
			base::data[2] = z;
		}

		// vec3(v, z);
		inline Tvec3(const Tvec2<T>& v, T z)
		{
			base::data[0] = v[0];
			base::data[1] = v[1];
			base::data[2] = z;
		}

		// vec3(x, v)
		inline Tvec3(T x, const Tvec2<T>& v)
		{
			base::data[0] = x;
			base::data[1] = v[0];
			base::data[2] = v[1];
		}
	};

	template <typename T>
	class Tvec4 : public vecN<T, 4>
	{
	public:
		typedef vecN<T, 4> base;

		// Uninitialized variable
		inline Tvec4() {}

		// Copy constructor
		inline Tvec4(const base& v) : base(v) {}

		// vec4(x, y, z, w);
		inline Tvec4(T x, T y, T z, T w)
		{
			base::data[0] = x;
			base::data[1] = y;
			base::data[2] = z;
			base::data[3] = w;
		}

		// vec4(v, z, w);
		inline Tvec4(const Tvec2<T>& v, T z, T w)
		{
			base::data[0] = v[0];
			base::data[1] = v[1];
			base::data[2] = z;
			base::data[3] = w;
		}

		// vec4(x, v, w);
		inline Tvec4(T x, const Tvec2<T>& v, T w)
		{
			base::data[0] = x;
			base::data[1] = v[0];
			base::data[2] = v[1];
			base::data[3] = w;
		}

		// vec4(x, y, v);
		inline Tvec4(T x, T y, const Tvec2<T>& v)
		{
			base::data[0] = x;
			base::data[1] = y;
			base::data[2] = v[0];
			base::data[3] = v[1];
		}

		// vec4(v1, v2);
		inline Tvec4(const Tvec2<T>& u, const Tvec2<T>& v)
		{
			base::data[0] = u[0];
			base::data[1] = u[1];
			base::data[2] = v[0];
			base::data[3] = v[1];
		}

		// vec4(v, w);
		inline Tvec4(const Tvec3<T>& v, T w)
		{
			base::data[0] = v[0];
			base::data[1] = v[1];
			base::data[2] = v[2];
			base::data[3] = w;
		}

		// vec4(x, v);
		inline Tvec4(T x, const Tvec3<T>& v)
		{
			base::data[0] = x;
			base::data[1] = v[0];
			base::data[2] = v[1];
			base::data[3] = v[2];
		}
	};

	// These types don't exist in GLSL and don't have full implementations
	// (constructors and such). This is enough to get some template functions
	// to compile correctly.
	typedef vecN<float, 1> vec1;
	typedef vecN<int, 1> ivec1;
	typedef vecN<unsigned int, 1> uvec1;
	typedef vecN<double, 1> dvec1;

	typedef Tvec2<float> vec2;
	typedef Tvec2<int> ivec2;
	typedef Tvec2<unsigned int> uvec2;
	typedef Tvec2<double> dvec2;

	typedef Tvec3<float> vec3;
	typedef Tvec3<int> ivec3;
	typedef Tvec3<unsigned int> uvec3;
	typedef Tvec3<double> dvec3;

	typedef Tvec4<float> vec4;
	typedef Tvec4<int> ivec4;
	typedef Tvec4<unsigned int> uvec4;
	typedef Tvec4<double> dvec4;

	template <typename T, int n>
	static inline const vecN<T, n> operator * (T x, const vecN<T, n>& v)
	{
		return v * x;
	}

	template <typename T>
	static inline const Tvec2<T> operator / (T x, const Tvec2<T>& v)
	{
		return div<Tvec2<T>, T>(x, v);
	}

	template <typename T>
	static inline const Tvec3<T> operator / (T x, const Tvec3<T>& v)
	{
		return div<Tvec3<T>, T>(x, v);
	}

	template <typename T>
	static inline const Tvec4<T> operator / (T x, const Tvec4<T>& v)
	{
		return div<Tvec4<T>, T>(x, v);
	}

	template <typename T, int len>
	static inline T dot(const vecN<T, len>& a, const vecN<T, len>& b)
	{
		return fast_dot<vecN<T, len>, T>(a, b);
	}

	template <typename T>
	static inline vecN<T, 3> cross(const vecN<T, 3>& a, const vecN<T, 3>& b)
	{
		return fast_cross(a, b);
	}

	template <typename T, int len>
	static inline T length(const vecN<T, len>& v)
	{
		T result = fast_dot<vecN<T, len>, T>(v, v);
		return (T)sqrt(result);
	}

	template <int len>
	static inline float fast_length(const vecN<float, len>& v)
	{
		return 1.f / (float)fastInvSqrt(lengthSquare(v));
	}

	template <typename T, int len>
	static inline T lengthSquare(const vecN<T, len>& v)
	{
		return fast_dot<vecN<T, len>, T>(v, v);
	}

	template <typename T, int len>
	static inline vecN<T, len> normalize(const vecN<T, len>& v)
	{
		return v / length(v);
	}

	template <int len>
	static inline vecN<float, len> normalize(const vecN<float, len>& v)
	{
		return v * fastInvSqrt(lengthSquare(v));
	}

	template <int len>
	static inline vecN<float, len> precise_normalize(const vecN<float, len>& v)
	{
		return v / length(v);
	}

	template <typename T, int len>
	static inline T distance(const vecN<T, len>& a, const vecN<T, len>& b)
	{
		return length(b - a);
	}

	template <typename T, int len>
	static inline T angle(const vecN<T, len>& a, const vecN<T, len>& b)
	{
		return arccos(dot(a, b));
	}

	template <typename T>
	class Tquaternion
	{
	public:
		inline Tquaternion()
		{

		}

		inline Tquaternion(const Tquaternion& q)
			: r(q.r),
			v(q.v)
		{

		}

		inline Tquaternion(T _r)
			: r(_r),
			v(T(0))
		{

		}

		inline Tquaternion(T _r, const Tvec3<T>& _v)
			: r(_r),
			v(_v)
		{

		}

		inline Tquaternion(const Tvec4<T>& _v)
			: r(_v[0]),
			v(_v[1], _v[2], _v[3])
		{
		}

		inline Tquaternion(T _x, T _y, T _z, T _w)
			: r(_x),
			v(_y, _z, _w)
		{

		}

		inline T& operator[](int n)
		{
			return a[n];
		}

		inline const T& operator[](int n) const
		{
			return a[n];
		}

		inline Tquaternion operator+(const Tquaternion& q) const
		{
			return quaternion(r + q.r, v + q.v);
		}

		inline Tquaternion& operator+=(const Tquaternion& q)
		{
			r += q.r;
			v += q.v;

			return *this;
		}

		inline Tquaternion operator-(const Tquaternion& q) const
		{
			return quaternion(r - q.r, v - q.v);
		}

		inline Tquaternion& operator-=(const Tquaternion& q)
		{
			r -= q.r;
			v -= q.v;

			return *this;
		}

		inline Tquaternion operator-() const
		{
			return Tquaternion(-r, -v);
		}

		inline Tquaternion operator*(const T s) const
		{
			return Tquaternion(a[0] * s, a[1] * s, a[2] * s, a[3] * s);
		}

		inline Tquaternion& operator*=(const T s)
		{
			r *= s;
			v *= s;

			return *this;
		}

		inline Tquaternion operator*(const Tquaternion& q) const
		{
			const T x1 = a[0];
			const T y1 = a[1];
			const T z1 = a[2];
			const T w1 = a[3];
			const T x2 = q.a[0];
			const T y2 = q.a[1];
			const T z2 = q.a[2];
			const T w2 = q.a[3];

			return Tquaternion(
				w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2,
				w1 * y2 + y1 * w2 + z1 * x2 - x1 * z2,
				w1 * z2 + z1 * w2 + x1 * y2 - y1 * x2,
				w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2
				);
		}

		inline Tquaternion operator/(const T s) const
		{
			return Tquaternion(a[0] / s, a[1] / s, a[2] / s, a[3] / s);
		}

		inline Tquaternion& operator/=(const T s)
		{
			r /= s;
			v /= s;

			return *this;
		}

		inline operator Tvec4<T>&()
		{
			return *(Tvec4<T>*)&a[0];
		}

		inline operator const Tvec4<T>&() const
		{
			return *(const Tvec4<T>*)&a[0];
		}

		inline bool operator==(const Tquaternion& q) const
		{
			return (r == q.r) && (v == q.v);
		}

		inline bool operator!=(const Tquaternion& q) const
		{
			return (r != q.r) || (v != q.v);
		}

		inline matNM<T, 4, 4> asMatrix() const
		{
			matNM<T, 4, 4> m;

			const T xx = x * x;
			const T yy = y * y;
			const T zz = z * z;
			const T ww = w * w;
			const T xy = x * y;
			const T xz = x * z;
			const T xw = x * w;
			const T yz = y * z;
			const T yw = y * w;
			const T zw = z * w;

			m[0][0] = T(1) - T(2) * (yy + zz);
			m[0][1] = T(2) * (xy - zw);
			m[0][2] = T(2) * (xz + yw);
			m[0][3] = T(0);

			m[1][0] = T(2) * (xy + zw);
			m[1][1] = T(1) - T(2) * (xx + zz);
			m[1][2] = T(2) * (yz - xw);
			m[1][3] = T(0);

			m[2][0] = T(2) * (xz - yw);
			m[2][1] = T(2) * (yz + xw);
			m[2][2] = T(1) - T(2) * (xx + yy);
			m[2][3] = T(0);

			m[3][0] = T(0);
			m[3][1] = T(0);
			m[3][2] = T(0);
			m[3][3] = T(1);

			return m;
		}

		/*
		inline T length() const
		{
		return vmath::length( Tvec4<T>(r, v) );
		}
		*/

	private:
		union
		{
			struct
			{
				T           r;
				Tvec3<T>    v;
			};
			struct
			{
				T           x;
				T           y;
				T           z;
				T           w;
			};
			T               a[4];
		};
	};

	typedef Tquaternion<float> quaternion;
	typedef Tquaternion<int> iquaternion;
	typedef Tquaternion<unsigned int> uquaternion;
	typedef Tquaternion<double> dquaternion;

	template <typename T>
	static inline Tquaternion<T> operator*(T a, const Tquaternion<T>& b)
	{
		return b * a;
	}

	template <typename T>
	static inline Tquaternion<T> operator/(T a, const Tquaternion<T>& b)
	{
		return Tquaternion<T>(a / b[0], a / b[1], a / b[2], a / b[3]);
	}

	template <typename T>
	static inline Tquaternion<T> normalize(const Tquaternion<T>& q)
	{
		return q / length(vecN<T, 4>(q));
	}

	template <>
	static inline Tquaternion<float> normalize(const Tquaternion<float>& q)
	{
		return q * fastInvSqrt(lengthSquare(vecN<float, 4>(q)));
	}

	template <typename T, const int w, const int h>
	class matNM
	{
	public:
		typedef class matNM<T, w, h> my_type;
		typedef class vecN<T, h> vector_type;

		// Default constructor does nothing, just like built-in types
		inline matNM()
		{
			// Uninitialized variable
		}

		// Copy constructor
		inline matNM(const matNM& that)
		{
			assign(that);
		}

		// Construction from element type
		// explicit to prevent assignment from T
		explicit inline matNM(T f)
		{
			for (int n = 0; n < w; n++)
			{
				data[n] = f;
			}
		}

		// Construction from vector
		inline matNM(const vector_type& v)
		{
			for (int n = 0; n < w; n++)
			{
				data[n] = v;
			}
		}

		// Assignment operator
		inline matNM& operator=(const my_type& that)
		{
			assign(that);
			return *this;
		}

		inline matNM operator+(const my_type& that) const
		{
			my_type result;
			int n;
			for (n = 0; n < w; n++)
				result.data[n] = data[n] + that.data[n];
			return result;
		}

		inline my_type& operator+=(const my_type& that)
		{
			return (*this = *this + that);
		}

		inline my_type operator-(const my_type& that) const
		{
			my_type result;
			int n;
			for (n = 0; n < w; n++)
				result.data[n] = data[n] - that.data[n];
			return result;
		}

		inline my_type& operator-=(const my_type& that)
		{
			return (*this = *this - that);
		}

		inline my_type operator*(const T& that) const
		{
			return mul<my_type>(that, *this);
		}

		inline my_type& operator*=(const T& that)
		{
			*this = mul<my_type&>(*this, that);
			return *this;
		}

		// Matrix multiply.
		// TODO: This only works for square matrices. Need more template skill to make a non-square version.
		inline my_type operator*(const my_type& that) const
		{
			static_assert(w == h, "square matrices is required");
#if USE_SIMD
			if (w != 4 || h != 4)
				return regular_mul(that);

			__m128 __result;

			GM_SIMD_float v[] = { data[0][0], data[0][1], data[0][2], data[0][3] };
			GM_SIMD_float row0[] = { that.data[0][0], that.data[0][1], that.data[0][2], that.data[0][3] };
			GM_SIMD_float row1[] = { that.data[1][0], that.data[1][1], that.data[1][2], that.data[1][3] };
			GM_SIMD_float row2[] = { that.data[2][0], that.data[2][1], that.data[2][2], that.data[2][3] };
			GM_SIMD_float row3[] = { that.data[3][0], that.data[3][1], that.data[3][2], that.data[3][3] };
			__m128	__v = _mm_load_ps(v);
			__m128	__row0 = _mm_load_ps(row0),
					__row1 = _mm_load_ps(row1),
					__row2 = _mm_load_ps(row2),
					__row3 = _mm_load_ps(row3);
			__m128	__x_mul_row0 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(0, 0, 0, 0)), __row0),
					__x_mul_row1 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(1, 1, 1, 1)), __row1),
					__x_mul_row2 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(2, 2, 2, 2)), __row2),
					__x_mul_row3 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(3, 3, 3, 3)), __row3);
			__result = _mm_add_ps(__x_mul_row0, __x_mul_row1);
			__result = _mm_add_ps(__result, __x_mul_row2);
			__result = _mm_add_ps(__result, __x_mul_row3);

			//TODO
			return regular_mul(that);

#else
			return regular_mul(that);
#endif
		}

		inline my_type& operator*=(const my_type& that)
		{
			return (*this = *this * that);
		}

		inline vector_type& operator[](int n) { return data[n]; }
		inline const vector_type& operator[](int n) const { return data[n]; }
		inline operator T*() { return &data[0][0]; }
		inline operator const T*() const { return &data[0][0]; }

		inline matNM<T, h, w> transpose(void) const
		{
			matNM<T, h, w> result;
			int x, y;

			for (y = 0; y < w; y++)
			{
				for (x = 0; x < h; x++)
				{
					result[x][y] = data[y][x];
				}
			}

			return result;
		}

		static inline my_type identity()
		{
			my_type result(0);

			for (int i = 0; i < w; i++)
			{
				result[i][i] = 1;
			}

			return result;
		}

		static inline int width(void) { return w; }
		static inline int height(void) { return h; }

	protected:
		// Column primary data (essentially, array of vectors)
		vecN<T, h> data[w];

		// Assignment function - called from assignment operator and copy constructor.
		inline void assign(const matNM& that)
		{
			int n;
			for (n = 0; n < w; n++)
				data[n] = that.data[n];
		}

		inline my_type regular_mul(const my_type& right) const
		{
			my_type result(0);
			for (int j = 0; j < w; j++)
			{
				for (int i = 0; i < w; i++)
				{
					T sum(0);
					for (int n = 0; n < w; n++)
					{
						sum += data[n][i] * right[j][n];
					}
					result[j][i] = sum;
				}
			}
			return result;
		}
	};

	/*
	template <typename T, const int N>
	class TmatN : public matNM<T,N,N>
	{
	public:
	typedef matNM<T,N,N> base;
	typedef TmatN<T,N> my_type;

	inline TmatN() {}
	inline TmatN(const my_type& that) : base(that) {}
	inline TmatN(float f) : base(f) {}
	inline TmatN(const vecN<T,4>& v) : base(v) {}

	inline my_type transpose(void)
	{
	my_type result;
	int x, y;

	for (y = 0; y < h; y++)
	{
	for (x = 0; x < h; x++)
	{
	result[x][y] = data[y][x];
	}
	}

	return result;
	}
	};
	*/

	template <typename T>
	class Tmat4 : public matNM<T, 4, 4>
	{
	public:
		typedef matNM<T, 4, 4> base;
		typedef Tmat4<T> my_type;

		inline Tmat4() {}
		inline Tmat4(const my_type& that) : base(that) {}
		inline Tmat4(const base& that) : base(that) {}
		inline Tmat4(const vecN<T, 4>& v) : base(v) {}
		inline Tmat4(const vecN<T, 4>& v0,
			const vecN<T, 4>& v1,
			const vecN<T, 4>& v2,
			const vecN<T, 4>& v3)
		{
			base::data[0] = v0;
			base::data[1] = v1;
			base::data[2] = v2;
			base::data[3] = v3;
		}
	};

	typedef Tmat4<float> mat4;
	typedef Tmat4<int> imat4;
	typedef Tmat4<unsigned int> umat4;
	typedef Tmat4<double> dmat4;

	template <typename T>
	class Tmat3 : public matNM<T, 3, 3>
	{
	public:
		typedef matNM<T, 3, 3> base;
		typedef Tmat3<T> my_type;

		inline Tmat3() {}
		inline Tmat3(const my_type& that) : base(that) {}
		inline Tmat3(const base& that) : base(that) {}
		inline Tmat3(const vecN<T, 3>& v) : base(v) {}
		inline Tmat3(const vecN<T, 3>& v0,
			const vecN<T, 3>& v1,
			const vecN<T, 3>& v2)
		{
			base::data[0] = v0;
			base::data[1] = v1;
			base::data[2] = v2;
		}
	};

	typedef Tmat3<float> mat3;
	typedef Tmat3<int> imat3;
	typedef Tmat3<unsigned int>umat3;
	typedef Tmat3<double> dmat3;

	template <typename T>
	class Tmat2 : public matNM<T, 2, 2>
	{
	public:
		typedef matNM<T, 2, 2> base;
		typedef Tmat2<T> my_type;

		inline Tmat2() {}
		inline Tmat2(const my_type& that) : base(that) {}
		inline Tmat2(const base& that) : base(that) {}
		inline Tmat2(const vecN<T, 2>& v) : base(v) {}
		inline Tmat2(const vecN<T, 2>& v0,
			const vecN<T, 2>& v1)
		{
			base::data[0] = v0;
			base::data[1] = v1;
		}
	};

	typedef Tmat2<float> mat2;

	static inline mat4 frustum(float left, float right, float bottom, float top, float n, float f)
	{
		mat4 result(mat4::identity());

		if ((right == left) ||
			(top == bottom) ||
			(n == f) ||
			(n < 0.0) ||
			(f < 0.0))
			return result;

		result[0][0] = (2.0f * n) / (right - left);
		result[1][1] = (2.0f * n) / (top - bottom);

		result[2][0] = (right + left) / (right - left);
		result[2][1] = (top + bottom) / (top - bottom);
		result[2][2] = -(f + n) / (f - n);
		result[2][3] = -1.0f;

		result[3][2] = -(2.0f * f * n) / (f - n);
		result[3][3] = 0.0f;

		return result;
	}

	static inline mat4 perspective(float fovy, float aspect, float n, float f)
	{
		float q = 1.0f / tan(radians(0.5f * fovy));
		float A = q / aspect;
		float B = (n + f) / (n - f);
		float C = (2.0f * n * f) / (n - f);

		mat4 result;

		result[0] = vec4(A, 0.0f, 0.0f, 0.0f);
		result[1] = vec4(0.0f, q, 0.0f, 0.0f);
		result[2] = vec4(0.0f, 0.0f, B, -1.0f);
		result[3] = vec4(0.0f, 0.0f, C, 0.0f);

		return result;
	}

	static inline mat4 ortho(float left, float right, float bottom, float top, float n, float f)
	{
		return mat4(vec4(2.0f / (right - left), 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 2.0f / (top - bottom), 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 2.0f / (n - f), 0.0f),
			vec4((left + right) / (left - right), (bottom + top) / (bottom - top), (n + f) / (f - n), 1.0f));
	}

	template <typename T>
	static inline Tmat4<T> translate(T x, T y, T z)
	{
		return Tmat4<T>(Tvec4<T>(1.0f, 0.0f, 0.0f, 0.0f),
			Tvec4<T>(0.0f, 1.0f, 0.0f, 0.0f),
			Tvec4<T>(0.0f, 0.0f, 1.0f, 0.0f),
			Tvec4<T>(x, y, z, 1.0f));
	}

	template <typename T>
	static inline Tmat4<T> translate(const vecN<T, 3>& v)
	{
		return translate(v[0], v[1], v[2]);
	}

	template <typename T>
	static inline Tmat4<T> lookat(const vecN<T, 3>& eye, const vecN<T, 3>& center, const vecN<T, 3>& up)
	{
		const Tvec3<T> f = normalize(center - eye);
		const Tvec3<T> upN = normalize(up);
		const Tvec3<T> s = normalize(cross(f, upN));
		const Tvec3<T> u = normalize(cross(s, f));
		const Tmat4<T> M = Tmat4<T>(Tvec4<T>(s[0], u[0], -f[0], T(0)),
			Tvec4<T>(s[1], u[1], -f[1], T(0)),
			Tvec4<T>(s[2], u[2], -f[2], T(0)),
			Tvec4<T>(T(0), T(0), T(0), T(1)));

		return M * translate<T>(-eye);
	}

	template <typename T>
	static inline Tmat4<T> scale(T x, T y, T z)
	{
		return Tmat4<T>(Tvec4<T>(x, 0.0f, 0.0f, 0.0f),
			Tvec4<T>(0.0f, y, 0.0f, 0.0f),
			Tvec4<T>(0.0f, 0.0f, z, 0.0f),
			Tvec4<T>(0.0f, 0.0f, 0.0f, 1.0f));
	}

	template <typename T>
	static inline Tmat4<T> scale(const Tvec3<T>& v)
	{
		return scale(v[0], v[1], v[2]);
	}

	template <typename T>
	static inline Tmat4<T> scale(T x)
	{
		return Tmat4<T>(Tvec4<T>(x, 0.0f, 0.0f, 0.0f),
			Tvec4<T>(0.0f, x, 0.0f, 0.0f),
			Tvec4<T>(0.0f, 0.0f, x, 0.0f),
			Tvec4<T>(0.0f, 0.0f, 0.0f, 1.0f));
	}

	template <typename T>
	static inline Tmat4<T> rotate(T angle, T x, T y, T z)
	{
		Tmat4<T> result;

		const T x2 = x * x;
		const T y2 = y * y;
		const T z2 = z * z;
		float rads = float(angle) * 0.0174532925f;
		const float c = cosf(rads);
		const float s = sinf(rads);
		const float omc = 1.0f - c;

		result[0] = Tvec4<T>(T(x2 * omc + c), T(y * x * omc + z * s), T(x * z * omc - y * s), T(0));
		result[1] = Tvec4<T>(T(x * y * omc - z * s), T(y2 * omc + c), T(y * z * omc + x * s), T(0));
		result[2] = Tvec4<T>(T(x * z * omc + y * s), T(y * z * omc - x * s), T(z2 * omc + c), T(0));
		result[3] = Tvec4<T>(T(0), T(0), T(0), T(1));

		return result;
	}

	template <typename T>
	static inline Tmat4<T> rotate(T angle, const vecN<T, 3>& v)
	{
		return rotate<T>(angle, v[0], v[1], v[2]);
	}

	template <typename T>
	static inline Tmat4<T> rotate(T angle_x, T angle_y, T angle_z)
	{
		return rotate(angle_z, 0.0f, 0.0f, 1.0f) *
			rotate(angle_y, 0.0f, 1.0f, 0.0f) *
			rotate(angle_x, 1.0f, 0.0f, 0.0f);
	}

#ifdef min
#undef min
#endif

	template <typename T>
	static inline T min(T a, T b)
	{
		return a < b ? a : b;
	}

#ifdef max
#undef max
#endif

	template <typename T>
	static inline T max(T a, T b)
	{
		return a >= b ? a : b;
	}

	template <typename T, const int N>
	static inline vecN<T, N> min(const vecN<T, N>& x, const vecN<T, N>& y)
	{
		vecN<T, N> t;
		int n;

		for (n = 0; n < N; n++)
		{
			t[n] = min(x[n], y[n]);
		}

		return t;
	}

	template <typename T, const int N>
	static inline vecN<T, N> max(const vecN<T, N>& x, const vecN<T, N>& y)
	{
		vecN<T, N> t;
		int n;

		for (n = 0; n < N; n++)
		{
			t[n] = max<T>(x[n], y[n]);
		}

		return t;
	}

	template <typename T, const int N>
	static inline vecN<T, N> clamp(const vecN<T, N>& x, const vecN<T, N>& minVal, const vecN<T, N>& maxVal)
	{
		return min<T>(max<T>(x, minVal), maxVal);
	}

	template <typename T, const int N>
	static inline vecN<T, N> smoothstep(const vecN<T, N>& edge0, const vecN<T, N>& edge1, const vecN<T, N>& x)
	{
		vecN<T, N> t;
		t = clamp((x - edge0) / (edge1 - edge0), vecN<T, N>(T(0)), vecN<T, N>(T(1)));
		return t * t * (vecN<T, N>(T(3)) - vecN<T, N>(T(2)) * t);
	}

	template <typename T, const int S>
	static inline vecN<T, S> reflect(const vecN<T, S>& I, const vecN<T, S>& N)
	{
		return I - 2 * dot(N, I) * N;
	}

	template <typename T, const int S>
	static inline vecN<T, S> refract(const vecN<T, S>& I, const vecN<T, S>& N, T eta)
	{
		T d = dot(N, I);
		T k = T(1) - eta * eta * (T(1) - d * d);
		if (k < 0.0)
		{
			return vecN<T, N>(0);
		}
		else
		{
			return eta * I - (eta * d + sqrt(k)) * N;
		}
	}

	template <typename T, const int N, const int M>
	static inline matNM<T, N, M> matrixCompMult(const matNM<T, N, M>& x, const matNM<T, N, M>& y)
	{
		matNM<T, N, M> result;
		int i, j;

		for (j = 0; j < M; ++j)
		{
			for (i = 0; i < N; ++i)
			{
				result[i][j] = x[i][j] * y[i][j];
			}
		}

		return result;
	}

	template <typename T, const int N, const int M>
	static inline vecN<T, N> operator*(const vecN<T, M>& vec, const matNM<T, N, M>& mat)
	{
		int n, m;
		vecN<T, N> result(T(0));

		for (m = 0; m < M; m++)
		{
			for (n = 0; n < N; n++)
			{
				result[n] += vec[m] * mat[n][m];
			}
		}

		return result;
	}

	template <typename T, const int N>
	static inline vecN<T, N> operator/(const T s, const vecN<T, N>& v)
	{
		int n;
		vecN<T, N> result;

		for (n = 0; n < N; n++)
		{
			result[n] = s / v[n];
		}

		return result;
	}

	/*
	template <typename T>
	static inline void quaternionToMatrix(const Tquaternion<T>& q, matNM<T,4,4>& m)
	{
	m[0][0] = q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3];
	m[0][1] = T(2) * (q[1] * q[2] + q[0] * q[3]);
	m[0][2] = T(2) * (q[1] * q[3] - q[0] * q[2]);
	m[0][3] = 0.0f;

	m[1][0] = T(2) * (q[1] * q[2] - q[0] * q[3]);
	m[1][1] = q[0] * q[0] - q[1] * q[1] + q[2] * q[2] - q[3] * q[3];
	m[1][2] = T(2) * (q[2] * q[3] + q[0] * q[1]);
	m[1][3] = 0.0f;

	m[2][0] = T(2) * (q[1] * q[3] + q[0] * q[2]);
	m[2][1] = T(2) * (q[2] * q[3] - q[0] * q[1]);
	m[2][2] = q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3];
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
	}
	*/

	template <typename T>
	static inline void quaternionToMatrix(const Tquaternion<T>& q, matNM<T, 4, 4>& m)
	{
		m = q.asMatrix();
	}

	template <typename T>
	static inline T mix(const T& A, const T& B, typename T::element_type t)
	{
		return B + t * (B - A);
	}

	template <typename T>
	static inline T mix(const T& A, const T& B, const T& t)
	{
		return B + t * (B - A);
	}

	static inline bool fuzzyCompare(float p1, float p2)
	{
		return (abs(p1 - p2) <= 0.01f);
	}

	static inline bool equals(const vmath::vec3& a, const vmath::vec3& b)
	{
		for (int i = 0; i < 3; i++)
		{
			if (!fuzzyCompare(a[i], b[i]))
				return false;
		}
		return true;
	}

	// regulars
	template <int len, typename T>
	struct regular
	{
	};

	template <typename T>
	struct regular<2, T>
	{
		inline static Tvec2<T> div(T x, const Tvec2<T>& v)
		{
			return Tvec2<T>(x / v[0], x / v[1]);
		}

		inline static Tvec2<T> div(const Tvec2<T>& v, T x)
		{
			return Tvec2<T>(v[0] / x, v[1] / x);
		}
	};

	template <typename T>
	struct regular<3, T>
	{
		inline static Tvec3<T> div(T x, const Tvec3<T>& v)
		{
			return Tvec3<T>(x / v[0], x / v[1], x / v[2]);
		}

		inline static Tvec3<T> div(const Tvec3<T>& v, T x)
		{
			return Tvec3<T>(v[0] / x, v[1] / x, v[2] / x);
		}
	};

	template <typename T>
	struct regular<4, T>
	{
		inline static Tvec4<T> div(T x, const Tvec4<T>& v)
		{
			return Tvec4<T>(x / v[0], x / v[1], x / v[2], x / v[3]);
		}

		inline static Tvec4<T> div(const Tvec4<T>& v, T x)
		{
			return Tvec4<T>(v[0] / x, v[1] / x, v[2] / x, v[3] / x);
		}
	};

};
#endif /* __VMATH_H__ */
