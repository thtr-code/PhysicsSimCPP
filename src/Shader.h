#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Shader{

public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath){
        //Read vertex and fragment glsl files
        string vertexCode, fragmentCode;
        ifstream fShaderFile, vShaderFile;
        vShaderFile.exceptions ( std::ifstream::badbit);
        fShaderFile.exceptions ( std::ifstream::badbit);
        string vline;
        string fline;

        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        if(!vShaderFile.is_open()){
            cerr << "Could not read the file";
        }
        while(getline(vShaderFile, vline)){
            vertexCode += vline + "\n";
        }
        if(!fShaderFile.is_open()){
            cerr << "Could not read the file";
        }
        while(getline(fShaderFile, fline)){
            fragmentCode += fline + "\n";
        }
        vShaderFile.close();
        fShaderFile.close();

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        //Compile retrieved shader files
        unsigned int vertex, fragment;
        int success;
        char infoLog[512];

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, nullptr); //Sends GLSL source text to GPU
        glCompileShader(vertex); //Compile into machine code for GPU
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
            cerr << "Vertex Shader Compilation Failed\n" << infoLog << std::endl;
        };

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, nullptr);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

        if(!success)
        {
            glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
            cerr << "Fragment Shader Compilation Failed\n" << infoLog << std::endl;
        };
        ID = glCreateProgram();

        glAttachShader(ID,vertex);
        glAttachShader(ID,fragment);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(ID, 512, nullptr, infoLog);
            cerr << "Linking Failed\n" << infoLog << std::endl;
        }
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }

    void use()const{
        glUseProgram(ID);
    }
    void setBool(const string &name, bool value) const{
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const string &name, int value) const{
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);

    }
    void setFloat(const string &name, float value) const{
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);

    }
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()),
                           1, GL_FALSE, glm::value_ptr(mat));
    }
    void setVec3(const std::string &name, const glm::vec3 &vec) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()),
                     1, glm::value_ptr(vec));
    }

};
