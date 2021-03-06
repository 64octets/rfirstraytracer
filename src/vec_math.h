#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <algorithm> // for min & max

typedef unsigned char byte;
const float epsilon = .001f;

// fast float random in interval -1,1
// source by RGBA: http://www.rgba.org/articles/sfrand/sfrand.htm
static inline
float sfrand()
{
	unsigned int a=(rand()<<16)|rand(); //we use the bottom 23 bits of the int, so one 16 bit rand() won't cut it.
	a=(a&0x007fffff) | 0x40000000;
	return( *((float*)&a) - 3.0f );
}

struct vec3
{
	float x,y,z;

	vec3() : x(0), y(0), z(0) {} // default
	vec3(float s) : x(s), y(s), z(s) {} // all same
	vec3(float x, float y, float z) : x(x), y(y), z(z) {} // just set xyz

	float dot(const vec3 b) const
	{
		const vec3 &a = *this; 
		return a.x*b.x + a.y*b.y + a.z*b.z;
	}

	vec3 cross(const vec3 &b) const
	{
		const vec3 &a = *this; 
		return vec3(
		 a.y * b.z - a.z * b.y,
		 a.z * b.x - a.x * b.z,
		 a.x * b.y - a.y * b.x
		 );
	}

	float length() const
	{
		return sqrt(x*x + y*y + z*z);
	}

	vec3 normalize()
	{
		float len = length();
		if ( len ) {
			float rlen = 1.0f / len;
			x *= rlen;
			y *= rlen;
			z *= rlen;
		}
		return *this;
	}

	float& operator[](int i)
	{
		if ( i==0 ) return x;
		if ( i==1 ) return y;
		else return z;
	}

	float operator[](int i) const
	{
		if ( i==0 ) return x;
		if ( i==1 ) return y;
		else return z;
	}

	vec3 operator- () const { return vec3(-x,-y,-z); }
	void operator+= (const vec3 &rhs) { x+=rhs.x; y+=rhs.y; z+=rhs.z; }
	void operator-= (const vec3 &rhs) { x-=rhs.x; y-=rhs.y; z-=rhs.z; }
	void operator*= (const vec3 &rhs) { x*=rhs.x; y*=rhs.y; z*=rhs.z; }
};

// Create binary, and two scalar overloads. This trick taken from cube2 src
#define VEC_OP(OP)                                                    \
static inline vec3 operator OP (const vec3 &a, const vec3 &b) {          \
	return vec3(a.x OP b.x, a.y OP b.y, a.z OP b.z);                   \
}                                                                     \
static inline vec3 operator OP (float s, const vec3 &b) {               \
	return vec3(s OP b.x, s OP b.y, s OP b.z);                         \
}                                                                     \
static inline vec3 operator OP (const vec3 &a, float s) {               \
	return vec3(a.x OP s, a.y OP s, a.z OP s);                         \
}
VEC_OP(*)
VEC_OP(/)
VEC_OP(+)
VEC_OP(-)
#undef VEC_OP

static inline
vec3 normalize(const vec3 &v)
{
	float len = v.length();
	if ( len ) {
		float rlen = 1.0f / len;
		return vec3(v.x*rlen, v.y*rlen, v.z*rlen);
	}
	return v;
}

static inline
vec3 cross(const vec3 &a, const vec3 &b)
{
	return vec3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
		);
}

// (u x v) dot w aka  Scalar Triple Product aka Box Product, corresponds to the 
// signed volume of a parallelepiped formed by three independant vectors (u,v,w)
// is equivalent to six times the volume of the tetrahedron spanned by (u,v,w)
static inline
float ScalarTriple(const vec3 &u, const vec3 &v, const vec3 &w)
{
	return cross(u,v).dot(w);
}

static inline
vec3 min(const vec3 &a, const vec3& b)
{
	return vec3( std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) );
}

static inline
vec3 max(const vec3 &a, const vec3& b)
{
	return vec3( std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) );
}

struct plane_t
{
	vec3 position;
	vec3 normal;
	float d; // Distance from origin

	plane_t() {};
	plane_t(const vec3 &position, const vec3 &normal) : position(position), normal(normal) 
	{
		d = -(normal.dot(position));
	}
};

enum object_type {
	SPHERE,
	PLANE
};

struct ray_t
{
	const vec3 origin;
	const vec3 dir;

	ray_t(const vec3 &origin, const vec3 &dir) : origin(origin), dir(dir) {}

	float intersectSphere( const vec3 &center, float radi, bool &isInside ) const
	{
		vec3 m = origin - center;
		float b = m.dot(dir);
		float c = m.dot(m) - radi*radi;
		// exit if r's origin is outside s (c>0) and r pointing away from s (b>0)
		if ( c > 0 && b > 0 ) return FLT_MAX;
		float discr = b*b - c;
		if ( discr < 0.0f ) return FLT_MAX;
		float t = -b - sqrt(discr);
		if ( t < 0.0f ) 
		{
			t = 0.0f; // clamp when inside sphere
			isInside = true;
		} else {
			isInside = false;
		}
		return t;
	}

	// doesn't calculate t, only tests if we hit or not
	float intersectSphereTest( const vec3 &center, float radi ) const
	{
		vec3 s = this->origin - center;
		float sv = s.dot(this->dir);
		float ss = s.dot(s);
		float discr = sv*sv - ss + radi*radi;
		if ( discr < 0.0f ) return FLT_MAX;;
		return discr;
	}

	float intersectPlane( const plane_t &p ) const
	{
		const ray_t &r = *this;
		const float nDotDir = p.normal.dot(r.dir);
		if ( nDotDir != 0.0f ) 
		{
			const float nDotO = p.normal.dot(r.origin);
			const float t = (p.d - nDotO) / nDotDir;
			if ( t < 0.f ) return FLT_MAX;
			return t;
		}
		return FLT_MAX;
	}

	float intersectSegmentPlane( const vec3 &a, const vec3 &b, const vec3 &c ) const
	{
		const vec3 n = (b-a).cross(c-a);
		const float d = n.dot(a);

		const ray_t &ray = *this;
		const float t = (d - n.dot(ray.origin)) / n.dot(ray.dir);
		if ( t < 0 ) return FLT_MAX;
		return t;
	}

	float intersectTriangle( const vec3 &a, const vec3 &b, const vec3 &c, vec3 &geometric_normal ) const
	{
		const vec3 p = origin;
		const vec3 q = origin + dir;
		const vec3 pq = q - p;
		const vec3 pa = a - p;
		const vec3 pb = b - p;
		const vec3 pc = c - p;
		// Test if pq is inside the edges bc, ca, and ab.
		// Done by testing that the signed tetrahedral volumes are all positive.
		float u = ScalarTriple(pq, pc, pb); if ( u < 0.0f ) return FLT_MAX;
		float v = ScalarTriple(pq, pa, pc); if ( v < 0.0f ) return FLT_MAX;
		float w = ScalarTriple(pq, pb, pa); if ( w < 0.0f ) return FLT_MAX;
		
		geometric_normal = normalize( (b-a).cross(c-a) );
		float t = intersectSegmentPlane(a,b,c); // Testing for inside triangle first is a little faster
		return t;
		
		// Compute barycentric coords (u,v,w) determining
		// intersection point r = u*a + v*b + w*c
		//float denom = 1.0f / (u+v+w);
		//u *= denom;
		//v *= denom;
		//w *= denom;
		//const vec3 pointOnTriangle = u*a + v*b + w*c;

	}

	// http://http.download.nvidia.com/developer/presentations/2005/GDC/Sponsored_Day/GDC_2005_VolumeRenderingForGames.pdf
	bool CheckBoxIntersection(const vec3 &boxmin, const vec3 &boxmax, float &tnear, float &tfar) const
	{
		const vec3 &rayOrigin = this->origin;
		const vec3 &rayDirNorm = this->dir;
        vec3 invR = 1.0f / rayDirNorm;
        vec3 tbot = invR * (boxmin - rayOrigin);
        vec3 ttop = invR * (boxmax - rayOrigin);

        //re-order intersections to find smallest and largest on each axis
        vec3 tmin = min(ttop, tbot);
        vec3 tmax = max(ttop, tbot);

        //find the largest tmin and the smallest tmax
        float t0xy = std::max(tmin.x, tmin.y);
        float t0xz = std::max(tmin.x, tmin.z);
        float largest_tmin = std::max(t0xy, t0xz);
        
		t0xy = std::min(tmax.x, tmax.y);
		t0xz = std::min(tmax.x, tmax.z);
        float smallest_tmax = std::min(t0xy, t0xz);

        //check for a hit
        if((largest_tmin > smallest_tmax)) {
                return false;
        } 

        tnear = largest_tmin;
        tfar = smallest_tmax;
        return true;
}
};

// fwd decl
class Material;

struct Intersection_t
{
	float t;
	vec3 normal;
	bool isInside;
	Material *mat;

	Intersection_t() : isInside(false)
	{
	}
};

struct Camera_t
{
	vec3 position;
	vec3 lookAt;
	vec3 lookUp;
};