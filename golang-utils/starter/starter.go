package starter

import (
	"context"
	"log/slog"
	"os"
	"os/signal"

	"github.com/eikcalb/misc/golang-utils/log"
)

// ApplicationRunnable represents a lambda that is invoked to run the application.
type ApplicationRunnable func(ctx context.Context, log *slog.Logger) error

func StartApplication(app ApplicationRunnable) {
	// create logger.
	appLog := log.CreateLog(slog.LevelDebug)

	// Create default context.
	ctx, cancel := context.WithCancel(context.Background())

	// Listen for signals to exit the application.
	ctx, stop := signal.NotifyContext(ctx, os.Interrupt, os.Kill)

	// Start the application in a separate thread.
	go func() {
		appLog.Info("Application Status: starting")

		err := app(ctx, appLog)
		if err != nil {
			appLog.Error("Failed to start application", "error", err)
			cancel()
		}

		appLog.Info("Application Status: stopped")
	}()

	<-ctx.Done()
	stop()
	appLog.Info("Cleanup complete")
}
