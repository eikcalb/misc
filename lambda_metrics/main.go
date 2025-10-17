package main

import (
	"context"
	"encoding/json"
	"errors"
	"flag"
	"fmt"

	"github.com/aws/aws-sdk-go-v2/aws"
	"github.com/aws/aws-sdk-go-v2/config"
	"github.com/aws/aws-sdk-go-v2/service/lambda"
)

type LambdaFunction struct {
	Arn     string
	Name    string
	Runtime string
}

var (
	cfg aws.Config
)

func lambda_list_functions() error {
	var err error
	client := lambda.NewFromConfig(cfg)

	lambdaFunctions := []LambdaFunction{}

	itemsPaginator := lambda.NewListFunctionsPaginator(client, &lambda.ListFunctionsInput{})

	if itemsPaginator.HasMorePages() {
		lambdas, err := itemsPaginator.NextPage(context.Background())
		if err != nil {
			fmt.Println("Error occured while retrieving lambdas")
			return err
		}

		for _, v := range lambdas.Functions {
			lambdaFunctions = append(lambdaFunctions, LambdaFunction{
				Arn:     *v.FunctionArn,
				Name:    *v.FunctionName,
				Runtime: string(v.Runtime),
			})
		}

	}

	if len(lambdaFunctions) > 0 {
		listJSON, err := json.Marshal(lambdaFunctions)
		if err != nil {
			return err
		}

		fmt.Println()
		fmt.Printf("%v\r\n", string(listJSON))

		return nil
	}

	err = errors.New("failed to retrieve list of lambda functions")
	return err
}

func main() {
	toolName := flag.String("tool", "", "Specify the tool to run")

	flag.Parse()

	switch *toolName {
	case "lambda":
		{
			var err error
			fmt.Println("Running lambda tool")
			err = lambda_list_functions()

			if err != nil {
				panic(err.Error())
			}
		}
	default:
		panic("Tool not found")
	}
}

func init() {
	_cfg, err := config.LoadDefaultConfig(context.Background())
	if err != nil {
		fmt.Printf("Error Initializing AWS: %v", err.Error())
		panic(1)
	}

	cfg = _cfg
}
