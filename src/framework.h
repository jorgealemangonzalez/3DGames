/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	Here we define all the mathematical classes like Vector3, Matrix44 and some extra useful geometrical functions
*/

#ifndef FRAMEWORK //macros to ensure the code is included once
#define FRAMEWORK

#include <vector>
#include <cmath>
#include <string>
#include <SFML/System/Vector3.hpp>

#define DEG2RAD 0.0174532925

class Vector2
{
public:
	union
	{
		struct { float x,y; };
		float value[2];
	};

	Vector2() { x = y = 0.0f; }
	Vector2(float x, float y) { this->x = x; this->y = y; }

	double length() { return sqrt(x*x + y*y); }
	double length() const { return sqrt(x*x + y*y); }

	float dot( const Vector2& v );
	float perpdot( const Vector2& v );

	void set(float x, float y) { this->x = x; this->y = y; }

	Vector2& normalize() { *this *= 1.0f / (float)length(); return *this; }
	Vector2 reverseY(float height) const { return Vector2(this->x, height-this->y); }

	float distance(const Vector2& v);
	void random(float range);

	void operator *= (float v) { x *= v; y *= v; }
	void operator /= (float v) { x /= v; y /= v; }
	bool operator != (Vector2 v) { return x == v.x && y == v.y;}
};

Vector2 operator * (const Vector2& a, float v);
Vector2 operator + (const Vector2& a, const Vector2& b);
Vector2 operator - (const Vector2& a, const Vector2& b);


class Vector3u
{
public:
	union
	{
		struct { unsigned int x;
			unsigned int y;
			unsigned int z; };
		unsigned int v[3];
	};
	Vector3u() { x = y = z = 0; }
	Vector3u(unsigned int x, unsigned int y, unsigned int z) { this->x = x; this->y = y; this->z = z; }
};

//*********************************

class Vector3
{
public:
	union
	{
		struct { float x,y,z; };
		float v[3];
	};

	Vector3() { x = y = z = 0.0f; }
	Vector3(float x, float y, float z) { this->x = x; this->y = y; this->z = z;	}
	Vector3(Vector2 v, float z=0) { this->x = v.x; this->y = v.y; this->z = z; }

	double length();
	double length() const;

	void set(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }

	Vector3& normalize();
	void random(float range);
	Vector3& random(Vector3 rangeMax, Vector3 rangeMin);

	float distance(const Vector3& v) const;

	Vector3 cross( const Vector3& v ) const;
	float dot( const Vector3& v ) const;

	void operator *= (float v) { x *= v; y *= v; z *= v; }
	void operator /= (float v) { x /= v; y /= v; z /= v; }
	void operator += (const Vector3& v) { x += v.x; y += v.y; z += v.z; }
	void operator -= (const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; }
	operator bool() const{
		return (x || y || z);
	}
	std::string toString() const;
	sf::Vector3<float> toMusicVector();
};

class Vector4
{
public:
	union
	{
		struct { float x,y,z,w; };
		float v[4];
	};

	Vector4() { x = y = z = w = 0.0f; }
	Vector4(Vector3 v, float w) { this->x = v.x; this->y = v.y; this->z = v.z; this->w = w; }
	Vector4(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w; }
	void set(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w; }
};

//****************************
//Matrix44 class
class Matrix44
{
public:

	//This matrix works in
	union { //allows to access the same var using different ways
		struct
		{
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;
		};
		float M[4][4]; //[row][column]
		float m[16];
	};

	Matrix44();
	Matrix44(const float* v);

	void multGL(); //multiply with opengl matrix
	void loadGL(); //load in opengl matrix
	Matrix44& clear();
	Matrix44& setIdentity();
	Matrix44& transpose();

	//get base vectors
	Vector3 rightVector() { return Vector3(m[0],m[1],m[2]); }
	Vector3 topVector() { return Vector3(m[4],m[5],m[6]); }
	Vector3 frontVector() { return Vector3(m[8],m[9],m[10]); }

	bool inverse();
	void setUpAndOrthonormalize(Vector3 up);
	void setFrontAndOrthonormalize(Vector3 front);

	Matrix44 getRotationOnly(); //used when having scale
	Vector3 getTranslationOnly(); //Get the translation vector

	//rotate only
	Vector3 rotateVector( const Vector3& v);

	//transform using world coordinates
	void traslate(float x, float y, float z);
	void scale(float x, float y, float z);
	void rotate( float angle_in_rad, const Vector3& axis  );

	//transform using local coordinates
	void traslateLocal(const Vector3& v);
	void traslateLocal(float x, float y, float z);
	void rotateLocal( float angle_in_rad, const Vector3& axis  );

	//create a transformation matrix from scratch
	void setTranslation(float x, float y, float z);
	void setTranslation(const Vector3& v);
	void setRotation( float angle_in_rad, const Vector3& axis );
	void setScale(float x, float y, float z);

	bool getXYZ(float* euler) const;

	//for cameras
	void lookAt(Vector3& eye, Vector3& center, Vector3& up);
	void perspective(float fov, float aspect, float near_plane, float far_plane);
	void ortho(float left, float right, float bottom, float top, float near_plane, float far_plane);

	Vector3 project(const Vector3& v);

	Matrix44 operator * (const Matrix44& matrix) const;
	std::string toString() const;
};

//Operators, they are our friends
//Matrix44 operator * ( const Matrix44& a, const Matrix44& b );
Vector3 operator * (const Matrix44& matrix, const Vector3& v);
Vector4 operator * (const Matrix44& matrix, const Vector4& v);
Vector3 operator * (const Vector3& a, const Vector3& b);
template <typename T> Vector3 operator * (const Vector3& a, T v);
Vector3 operator + (const Vector3& a, const Vector3& b);
Vector3 operator - (const Vector3& a, const Vector3& b);
Vector3 operator - (const Vector3& a);
std::ostream& operator<<(std::ostream& out, const Vector3 &v);
std::ostream& operator<<(std::ostream &out, const Matrix44& m);

Vector3 mix(const Vector3& a, const Vector3& b, float& f);

float ComputeSignedAngle( Vector2 a, Vector2 b);
Vector3 RayPlaneCollision( const Vector3& plane_pos, const Vector3& plane_normal, const Vector3& ray_origin, const Vector3& ray_dir );
bool RaySphereCollision(const Vector3& center, float radius, const Vector3& ray_origin, const Vector3& ray_dir, Vector3& result );

//float random();

#endif