package main

import (
	"ingress-status-poller/poller"

	"github.com/eikcalb/misc/golang-utils/env"
	"github.com/eikcalb/misc/golang-utils/starter"
)

func main() {
	env.LoadDotEnv()
	starter.StartApplication(poller.PollIngress)
}
