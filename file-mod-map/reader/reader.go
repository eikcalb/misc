package reader

import (
	"encoding/json"
	"errors"
	"file-mod-map/types"
	"flag"
	"io/fs"
	"log/slog"
	"os"
	"path/filepath"
	"slices"
	"strings"
)

func ReadModeMap(args []string, logger *slog.Logger) error {
	omittedDirs := make([]string, 0)
	modeMap := make(types.FilePermMap)

	fSet := flag.NewFlagSet("write", flag.ExitOnError)

	rootDir := fSet.String("root", "", "Root directory to start crawling from.")
	omitDirSuffix := fSet.String("omit", "", "Specify a comma separated list of directories to omit")
	inputFileName := fSet.String("input", "output.json", "Input file path to store the file mode map")
	ignoreInaccessibleFiles := fSet.Bool("ignore", false, "Input file path to store the file mode map")

	fSet.Parse(args)

	if len(*rootDir) == 0 {
		logger.Error("Root directory must be specified")
		return errors.New("failed to set root directory")
	}

	if len(*omitDirSuffix) != 0 {
		omittedDirs = append(omittedDirs, strings.Split(*omitDirSuffix, ",")...)
		logger.Debug("Omitting the directories with the following suffices", "dirs", omittedDirs)
	}

	inputData, err := os.ReadFile(*inputFileName)
	if err != nil {
		logger.Error("Failed to read mode map", "cause", err)
		return errors.Join(errors.New("failed to read mode map"), err)
	}

	json.Unmarshal(inputData, &modeMap)

	walkFunc := func(path string, entry fs.DirEntry, err error) error {
		if err != nil && !*ignoreInaccessibleFiles {
			logger.Error("Error occurred while reading path", "cause", err, "path", path)
			return err
		}

		if entry.IsDir() && slices.Contains(omittedDirs, filepath.Base(path)) {
			logger.Info("Skipping directory", "path", path)
			return fs.SkipDir
		}

		if _, ok := modeMap[path]; !ok {
			logger.Info("Path does not exist in source", "path", path)
			return nil
		}

		targetMode := modeMap[path]
		err = os.Chmod(path, targetMode)
		if err != nil && !*ignoreInaccessibleFiles {
			logger.Error("Failed to change file mode", "cause", err, "path", path, "entry", entry)
			return errors.Join(errors.New("failed to change file mode"), err)
		}

		return nil
	}

	err = fs.WalkDir(os.DirFS(*rootDir), ".", walkFunc)
	if err != nil {
		logger.Error("Failed to walk directory", "cause", err)
		return errors.Join(errors.New("failed to walk directory"), err)
	}

	parsedCount := len(modeMap)
	logger.Info("Parsing complete", "count", parsedCount)

	logger.Info("Successfully processed file mode map", "path", inputFileName)
	return nil
}
