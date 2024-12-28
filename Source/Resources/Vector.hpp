#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Vector2 {
	float x, y;

	//Default constructor
	Vector2(const float x, const float y) {
		this->x = x;
		this->y = y;
	}

	Vector2(const Vector2 &vec2) {
		x = vec2.x;
		y = vec2.y;
	}

	//Retrieval funcs
	Vector2 xy() const {
		return Vector2(x,y);
	}

	Vector2 yx() const {
		return Vector2(y,x);
	}

	glm::vec2 glm() {
		return glm::vec2(x,y);
	}

	static Vector2 fromGLM(const glm::vec2 vec2) {
		return Vector2(vec2.x, vec2.y);
	}

	//Equality operator
	bool operator == (const Vector2 &vec) const {
		return (x == vec.x && y == vec.y);
	}

	//Scalar operators
	Vector2 operator + (const float a) const {
		return Vector2(x + a, y + a);
	}

	Vector2 operator - (const float a) const {
		return Vector2(x - a, y - a);
	}

	Vector2 operator * (const float a) const {
		return Vector2(x * a, y * a);
	}

	Vector2 operator / (const float a) const {
		return Vector2(x / a, y / a);
	}

	//Vector operators
	Vector2 operator + (const Vector2 &vec) const {
		return Vector2(x + vec.x, y + vec.y);
	}

	Vector2 operator - (const Vector2 &vec) const {
		return Vector2(x - vec.x, y - vec.y);
	}

	Vector2 operator * (const Vector2 &vec) const {
		return Vector2(x * vec.x, y * vec.y);
	}

	Vector2 operator / (const Vector2 &vec) const {
		return Vector2(x / vec.x, y / vec.y);
	}

	Vector2 operator -() const {
		return Vector2(-x, -y);
	}
};

//Vector2 ostream operator
inline std::ostream& operator << (std::ostream& os, const Vector2& vec) {
	os << "(" << vec.x << ", " << vec.y << ")";
	return os;
}

struct Vector3 {
	float x = 0, y = 0, z = 0;

	Vector3(const float x, const float y, const float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	//Vector constuctors
	Vector3(const Vector2 vec2, const float z) {
		x = vec2.x;
		y = vec2.y;
		this->z = z;
	}

	Vector3(const float x, const Vector2 vec2) {
		this->x = x;
		y = vec2.x;
		z = vec2.y;
	}

	Vector3(const Vector3 &vec3) {
		x = vec3.x;
		y = vec3.y;
		z = vec3.z;
	}

	//Retrieval operators
	Vector2 xy() const {
		return Vector2(x,y);
	}

	Vector2 yz() const {
		return Vector2(y,z);
	}

	Vector2 xz() const {
		return Vector2(x,z);
	}

	Vector3 xyz() const {
		return Vector3(x,y,z);
	}

	Vector3 zyx() const {
		return Vector3(z,y,x);
	}

	//Glm conversion
	glm::vec3 glm() {
		return glm::vec3(x,y,z);
	}

	static Vector3 fromGLM(const glm::vec3 vec3) {
		return Vector3(vec3.x, vec3.y, vec3.z);
	}

	//Equality operator
	bool operator == (const Vector3 &vec) const {
		return (x == vec.x && y == vec.y && z == vec.z);
	}

	//Scalar operators
	Vector3 operator + (const float a) const {
		return Vector3(x + a, y + a, z + a);
	}

	Vector3 operator - (const float a) const {
		return Vector3(x - a, y - a, z - a);
	}

	Vector3 operator * (const float a) const {
		return Vector3(x * a, y * a, z * a);
	}

	Vector3 operator / (const float a) const {
		return Vector3(x / a, y / a, z / a);
	}

	//Vector operators
	Vector3 operator + (const Vector3 &vec) const {
		return Vector3(x + vec.x, y + vec.y, z + vec.z);
	}

	Vector3 operator - (const Vector3 &vec) const {
		return Vector3(x - vec.x, y - vec.y, z - vec.z);
	}

	Vector3 operator * (const Vector3 &vec) const {
		return Vector3(x * vec.x, y * vec.y, z * vec.z);
	}

	Vector3 operator / (const Vector3 &vec) const {
		return Vector3(x / vec.x, y / vec.y, z / vec.z);
	}

	Vector3 operator -() const {
		return Vector3(-x, -y, -z);
	}
};

//Vector3 ostream operator
inline std::ostream& operator << (std::ostream& os, const Vector3& vec) {
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return os;
}

struct Vector4 {
	float x, y, z, w;

	//Accessors
	Vector2 xy() const {
		return Vector2(x,y);
	}

	//Default constructor
	Vector4(const float x, const float y, const float z, const float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	//Vector2 constructors
	Vector4(Vector2 vec2, const float z, const float w) {
		x = vec2.x;
		y = vec2.y;
		this->z = z;
		this->w = w;
	}

	Vector4(const float x, Vector2 vec2, const float w) {
		this->x = x;
		y = vec2.x;
		z = vec2.y;
		this->w = w;
	}

	Vector4(const float x, const float y, Vector2 vec2) {
		this->x = x;
		this->y = y;
		z = vec2.x;
		w = vec2.y;
	}

	Vector4(const Vector2 vec2a, const Vector2 vec2b) {
		x = vec2a.x;
		y = vec2a.y;
		z = vec2b.x;
		w = vec2b.y;
	}

	//Vector3 constructors
	Vector4(Vector3 vec3, const float w) {
		x = vec3.x;
		y = vec3.y;
		z = vec3.z;
		this->w = w;
	}

	Vector4(const float x, const Vector3 vec3) {
		this->x = x;
		y = vec3.x;
		z = vec3.y;
		w = vec3.z;
	}

	Vector2 yz() const {
		return Vector2(y,z);
	}

	Vector2 zw() const {
		return Vector2(z,w);
	}

	Vector2 xz() const {
		return Vector2(x,z);
	}

	Vector2 yw() const {
		return Vector2(y,w);
	}

	Vector2 xw() const {
		return Vector2(x,w);
	}

	Vector3 xyz() const {
		return Vector3(x,y,z);
	}

	Vector3 yzw() const {
		return Vector3(y,z,w);
	}

	Vector3 xyw() const {
		return Vector3(x,y,w);
	}

	Vector3 xzw() const {
		return Vector3(x,z,w);
	}

	Vector4 xyzw() const {
		return Vector4(x,y,z,w);
	}

	Vector4 wzxy() const {
		return Vector4(w,z,y,x);
	}

	//Glm conversion
	glm::vec4 glm() const {
		return glm::vec4(x,y,z,w);
	}

	static Vector4 fromGLM(const glm::vec4 vec) {
		return Vector4(vec.x, vec.y, vec.z, vec.w);
	}

	//Scalar operators
	Vector4 operator + (const float a) const {
		return Vector4(x + a, y + a, z + a, w + a);
	}

	Vector4 operator - (const float a) const {
		return Vector4(x - a, y - a, z - a, w - a);
	}

	Vector4 operator * (const float a) const {
		return Vector4(x * a, y * a, z * a, w * a);
	}

	Vector4 operator / (const float a) const {
		return Vector4(x / a, y / a, z / a, w / a);
	}

	//Vector operators
	Vector4 operator + (const Vector4 &vec) const {
		return Vector4(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
	}

	Vector4 operator - (const Vector4 &vec) const {
		return Vector4(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
	}

	Vector4 operator * (const Vector4 &vec) const {
		return Vector4(x * vec.x, y * vec.y, z * vec.z, w * vec.w);
	}

	Vector4 operator / (const Vector4 &vec) const {
		return Vector4(x / vec.x, y / vec.y, z / vec.z, w / vec.w);
	}

	Vector4 operator -() const {
		return Vector4(-x, -y, -z, -w);
	}
};

//Vector3 ostream operator
inline std::ostream& operator << (std::ostream& os, const Vector4& vec) {
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	return os;
}

#endif //VECTOR_HPP
