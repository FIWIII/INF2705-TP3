#ifndef SHADER_STORAGE_BUFFER_H
#define SHADER_STORAGE_BUFFER_H

#include <glbinding/gl/gl.h>

using namespace gl;

class ShaderStorageBuffer
{
public:
    ShaderStorageBuffer();
    ShaderStorageBuffer(ShaderStorageBuffer&& other);
    ~ShaderStorageBuffer();
    
    void allocate(const void* data, GLsizeiptr byteSize, GLenum usage);
    
    void setBindingIndex(GLuint index);

    void updateData(const void* data, GLintptr offset, GLsizeiptr byteSize);
    
    void bindAsArray();
    
    ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other);
    
private:
    GLuint id_;
};

#endif // SHADER_STORAGE_BUFFER_H
