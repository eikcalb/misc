package env

import "github.com/joho/godotenv"

// LoadDotEnv parses the environment variables in the files specified and makes these
// variables available to the process.
func LoadDotEnv(filenames ...string) {
	godotenv.Load(filenames...)
}
