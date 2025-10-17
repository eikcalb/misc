package writer

import (
	"encoding/json"
	"errors"
	"flag"
	"io/fs"
	"log/slog"
	"os"
	"path/filepath"
	"slices"
	"strings"

	"file-mod-map/types"
)

func WriteModeMap(args []string, logger *slog.Logger) error {
	omittedDirs := make([]string, 0)
	modeMap := make(types.FilePermMap)

	fSet := flag.NewFlagSet("write", flag.ExitOnError)

	rootDir := fSet.String("root", "", "Root directory to start crawling from.")
	omitDirSuffix := fSet.String("omit", "", "Specify a comma separated list of directories to omit")
	outputFileName := fSet.String("output", "output.json", "Output file path to store the file mode map")

	fSet.Parse(args)

	if len(*rootDir) == 0 {
		logger.Error("Root directory must be specified")
		return errors.New("failed to set root directory")
	}

	if len(*omitDirSuffix) != 0 {
		omittedDirs = append(omittedDirs, strings.Split(*omitDirSuffix, ",")...)
		logger.Debug("Omitting the directories with the following suffices", "dirs", omittedDirs)
	}

	walkFunc := func(path string, entry fs.DirEntry, err error) error {
		if err != nil {
			logger.Error("Error occurred while reading path", "cause", err, "path", path)
			return err
		}

		if entry.IsDir() && slices.Contains(omittedDirs, filepath.Base(path)) {
			logger.Info("Skipping directory", "path", path)
			return fs.SkipDir
		}

		info, err := entry.Info()
		if err != nil {
			logger.Error("Failed to stat path", "path", path)
			return errors.New("failed to stat path")
		}

		modeMap[path] = info.Mode().Perm()
		return nil
	}

	err := fs.WalkDir(os.DirFS(*rootDir), ".", walkFunc)
	if err != nil {
		logger.Error("Failed to walk directory", "cause", err)
		return errors.New("failed to walk directory")
	}

	parsedCount := len(modeMap)
	logger.Info("Parsing complete", "count", parsedCount)

	encoded, err := json.Marshal(modeMap)
	if err != nil {
		logger.Error("Failed to encode mode map", "cause", err)
		return errors.New("failed to encode mode map")
	}

	err = os.WriteFile(*outputFileName, encoded, 0644)
	if err != nil {
		logger.Error("Failed to write output", "cause", err)
		return errors.New("failed to write output")
	}

	logger.Info("Successfully generated file mode map", "path", outputFileName)
	return nil
}
