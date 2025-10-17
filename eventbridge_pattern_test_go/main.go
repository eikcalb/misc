package main

import "os"

const (
	EXIT_SUCCESS = 0

	EXIT_FAIL = 1
)

func main() {
	os.Exit(EXIT_SUCCESS)
}
