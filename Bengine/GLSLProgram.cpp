#include "GLSLProgram.h"
#include "BengineErrors.h"

#include <vector>

#include <fstream>

namespace Bengine {

    //The : _numAttributes(0) ect. is an initialization list. It is a better way to initialize variables, since it avoids an extra copy. 
    GLSLProgram::GLSLProgram() : _numAttributes(0), _programID(0), _vertexShaderID(0), _fragmentShaderID(0)
    {

    }


    GLSLProgram::~GLSLProgram()
    {
    }

    //Compiles the shaders into a form that your GPU can understand
    void GLSLProgram::compileShaders(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilepath) {
        //Vertex and fragment shaders are successfully compiled.
        //Now time to link them together into a program.
        //Get a program object.
        _programID = glCreateProgram();

        //Create the vertex shader object, and store its ID
        _vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        if (_vertexShaderID == 0) {
            fatalError("Vertex shader failed to be created!");
        }

        //Create the fragment shader object, and store its ID
        _fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        if (_fragmentShaderID == 0) {
            fatalError("Fragment shader failed to be created!");
        }

        //Compile each shader
        compileShader(vertexShaderFilePath, _vertexShaderID);
        compileShader(fragmentShaderFilepath, _fragmentShaderID);
    }

    void GLSLProgram::linkShaders() {

        //Attach our shaders to our program
        glAttachShader(_programID, _vertexShaderID);
        glAttachShader(_programID, _fragmentShaderID);

        //Link our program
        glLinkProgram(_programID);

        //Note the different functions here: glGetProgram* instead of glGetShader*.
        GLint isLinked = 0;
        glGetProgramiv(_programID, GL_LINK_STATUS, (int *)&isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(_programID, GL_INFO_LOG_LENGTH, &maxLength);

            //The maxLength includes the NULL character
            std::vector<char> errorLog(maxLength);
            glGetProgramInfoLog(_programID, maxLength, &maxLength, &errorLog[0]);



            //We don't need the program anymore.
            glDeleteProgram(_programID);
            //Don't leak shaders either.
            glDeleteShader(_vertexShaderID);
            glDeleteShader(_fragmentShaderID);

            //print the error log and quit
            std::printf("%s\n", &(errorLog[0]));
            fatalError("Shaders failed to link!");
        }

        //Always detach shaders after a successful link.
        glDetachShader(_programID, _vertexShaderID);
        glDetachShader(_programID, _fragmentShaderID);
        glDeleteShader(_vertexShaderID);
        glDeleteShader(_fragmentShaderID);
    }

    //Adds an attribute to our shader. SHould be called between compiling and linking.
    void GLSLProgram::addAttribute(const std::string& attributeName) {
        glBindAttribLocation(_programID, _numAttributes++, attributeName.c_str());
    }

    GLint GLSLProgram::getUniformLocation(const std::string& uniformName) {
        GLint location = glGetUniformLocation(_programID, uniformName.c_str());
        if (location == GL_INVALID_INDEX) {
            fatalError("Uniform " + uniformName + " not found in shader!");
        }
        return location;
    }

    //enable the shader, and all its attributes
    void GLSLProgram::use() {
        glUseProgram(_programID);
        //enable all the attributes we added with addAttribute
        for (int i = 0; i < _numAttributes; i++) {
            glEnableVertexAttribArray(i);
        }
    }

    //disable the shader
    void GLSLProgram::unuse() {
        glUseProgram(0);
        for (int i = 0; i < _numAttributes; i++) {
            glDisableVertexAttribArray(i);
        }
    }

    //Compiles a single shader file
    void GLSLProgram::compileShader(const std::string& filePath, GLuint id) {

        //Open the file
        std::ifstream shaderFile(filePath);
        if (shaderFile.fail()) {
            perror(filePath.c_str());
            fatalError("Failed to open " + filePath);
        }

        //File contents stores all the text in the file
        std::string fileContents = "";
        //line is used to grab each line of the file
        std::string line;

        //Get all the lines in the file and add it to the contents
        while (std::getline(shaderFile, line)) {
            fileContents += line + "\n";
        }

        shaderFile.close();

        //get a pointer to our file contents c string;
        const char* contentsPtr = fileContents.c_str();
        //tell opengl that we want to use fileContents as the contents of the shader file
        glShaderSource(id, 1, &contentsPtr, nullptr);

        //compile the shader
        glCompileShader(id);

        //check for errors
        GLint success = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);

        if (success == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

            //The maxLength includes the NULL character
            std::vector<char> errorLog(maxLength);
            glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);

            //Provide the infolog in whatever manor you deem best.
            //Exit with failure.
            glDeleteShader(id); //Don't leak the shader.

            //Print error log and quit
            std::printf("%s\n", &(errorLog[0]));
            fatalError("Shader " + filePath + " failed to compile");
        }
    }

}