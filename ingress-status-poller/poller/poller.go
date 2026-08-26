package poller

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"errors"
	"flag"
	"fmt"
	"log/slog"
	"net"
	"net/http"
	"net/url"
	"os"
	"strings"
	"time"
)

const (
	ENVIRONMENT_INT  = "int"
	ENVIRONMENT_TEST = "test"

	// Production environment.
	ENVIRONMENT_LIVE = "live"
)

const (
	ENV_INGRESS_ENDPOINT_PATTERN = "INGRESS_ENDPOINT_PATTERN"

	CA_PATH       = "/etc/pki/tls/certs/trust.pem"
	CERT_PATH     = "/etc/pki/tls/certs/client.crt"
	CERT_KEY_PATH = "/etc/pki/tls/private/client.key"

	FAIL    = "❌"
	SUCCESS = "✅"
)

type PollerConfig struct {
	endpoint *url.URL
	ticker   *time.Ticker
}

func getConfig() (c *PollerConfig, err error) {
	fs := flag.NewFlagSet("poller", flag.ExitOnError)
	interval := fs.Duration("d", time.Duration(5)*time.Second, "Polling interval duration")
	environment := fs.String("e", "", "Ingress endpoint environment")

	err = fs.Parse(os.Args[1:])
	if err != nil {
		fs.PrintDefaults()
		return nil, err
	}

	env := strings.TrimSpace(*environment)
	if env != ENVIRONMENT_INT && env != ENVIRONMENT_TEST && env != ENVIRONMENT_LIVE {
		fs.PrintDefaults()

		errMsg := fmt.Sprintf("a valid environment must be provided, e.g. %s, %s or %s", ENVIRONMENT_INT, ENVIRONMENT_TEST, ENVIRONMENT_LIVE)

		return nil, errors.New(errMsg)
	}

	ingressEndpointFormat := os.Getenv(ENV_INGRESS_ENDPOINT_PATTERN)
	if len(ingressEndpointFormat) == 0 {
		return nil, errors.New("ingress endpoint format must be provided")
	}

	ingressEndpoint := fmt.Sprintf(os.Getenv(ENV_INGRESS_ENDPOINT_PATTERN), env)
	parsedURL, err := url.Parse(ingressEndpoint)
	if err != nil {
		return nil, errors.New("failed to parse Ingress endpoint")

	}

	ticker := time.NewTicker(*interval)

	return &PollerConfig{endpoint: parsedURL, ticker: ticker}, err
}

func pingIngress(log *slog.Logger, ingressEndpoint *url.URL) {
	caCert, err := os.ReadFile(CA_PATH)
	if err != nil {
		log.Error(FAIL)
		log.Debug("Failed to read CA", "cause", err)
		return
	}

	caCertPool := x509.NewCertPool()
	ok := caCertPool.AppendCertsFromPEM(caCert)
	if !ok {
		log.Error(FAIL)
		log.Debug("Failed to append CA", "cause", err)
		return
	}

	cert, err := tls.LoadX509KeyPair(CERT_PATH, CERT_KEY_PATH)
	if err != nil {
		log.Error(FAIL)
		log.Debug("Failed to read certs", "cause", err)
		return
	}

	client := &http.Client{
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{
				RootCAs:      caCertPool,
				Certificates: []tls.Certificate{cert},
			},
		},
	}

	ips, err := net.LookupHost(ingressEndpoint.Hostname())
	if err != nil {
		log.Error(FAIL)
		log.Debug("Failed to resolve Ingress endpoint IP", "cause", err)
		return
	}

	resp, err := client.Get(ingressEndpoint.String())
	if err != nil || resp.StatusCode != 200 {
		log.Error(FAIL)
		log.Debug("Failed to contact Ingress", "cause", err, "ips", ips)
		return
	}

	log.Info(SUCCESS, "ips", ips)
	resp.Body.Close()
}

// PollIngress will make continuous requests to the ingress endpoint and when
// the servce is down.
func PollIngress(ctx context.Context, log *slog.Logger) (err error) {
	config, err := getConfig()
	if err != nil {
		log.Error("Failed to retrieve config", "cause", err)
		return err
	}

	log.Info(fmt.Sprintf("Starting poller for endpoint: %s", config.endpoint.String()))

	for range config.ticker.C {
		go pingIngress(log, config.endpoint)
	}

	return err
}
