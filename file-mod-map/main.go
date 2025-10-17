package main

import (
	"file-mod-map/reader"
	"file-mod-map/writer"
	"io/fs"
	"log/slog"
	"os"

	"github.com/eikcalb/misc/golang-utils/log"
)

type FilePermMap map[string]fs.FileMode

func main() {
	logger := log.CreateLog(slog.LevelDebug)

	command := os.Args[1]

	switch command {
	case "read":
		err := reader.ReadModeMap(os.Args[2:], logger)
		if err != nil {
			os.Exit(1)
		}
	case "write":
		err := writer.WriteModeMap(os.Args[2:], logger)
		if err != nil {
			os.Exit(1)
		}
	default:
		logger.Error("Command not found", "command", command)
		os.Exit(1)
	}
}
