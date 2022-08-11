glew = {
	source = path.join(dependencies.basePath, "glew"),
}

function glew.import()
	links { "glew", "OpenGL32" }
	glew.includes()
end

function glew.includes()
	includedirs { path.join(glew.source, "include") }

	defines {
		"GLEW_STATIC",
	}
end

function glew.project()
	project "glew"
		language "C"

		glew.includes()
		files
		{
			path.join(glew.source, "src/glew.c")
		}
		
		defines {
			"GLEW_BUILD"
		}

		removelinks "*"
		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, glew)
