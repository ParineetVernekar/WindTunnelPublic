import { mqtt5, auth, iot } from "aws-iot-device-sdk-v2";
import { once } from "events";
import { fromCognitoIdentityPool } from "@aws-sdk/credential-providers";
import { CognitoIdentityCredentials } from "@aws-sdk/credential-provider-cognito-identity/dist-types/fromCognitoIdentity"
import { toUtf8 } from "@aws-sdk/util-utf8-browser";
import { useState, useRef } from 'react';
// @ts-ignore
import { AWS_REGION, AWS_COGNITO_IDENTITY_POOL_ID, AWS_IOT_ENDPOINT } from './settings';
import jquery from "jquery";

const $: JQueryStatic = jquery;


function log(msg: string) {
    // let now = new Date();
    // $('#message').append(`<pre>${now.toString()}: ${msg}</pre>`);
}

/**
 * AWSCognitoCredentialOptions. The credentials options used to create AWSCongnitoCredentialProvider.
 */
interface AWSCognitoCredentialOptions {
    IdentityPoolId: string,
    Region: string
}

/**
 * AWSCognitoCredentialsProvider. The AWSCognitoCredentialsProvider implements AWS.CognitoIdentityCredentials.
 *
 */
class AWSCognitoCredentialsProvider extends auth.CredentialsProvider {
    private options: AWSCognitoCredentialOptions;
    private cachedCredentials?: CognitoIdentityCredentials;

    constructor(options: AWSCognitoCredentialOptions, expire_interval_in_ms?: number) {
        super();
        this.options = options;

        setInterval(async () => {
            await this.refreshCredentials();
        }, expire_interval_in_ms ?? 3600 * 1000);
    }

    getCredentials(): auth.AWSCredentials {
        return {
            aws_access_id: this.cachedCredentials?.accessKeyId ?? "",
            aws_secret_key: this.cachedCredentials?.secretAccessKey ?? "",
            aws_sts_token: this.cachedCredentials?.sessionToken,
            aws_region: this.options.Region
        }
    }

    async refreshCredentials() {
        log('Fetching Cognito credentials');
        this.cachedCredentials = await fromCognitoIdentityPool({
            // Required. The unique identifier for the identity pool from which an identity should be
            // retrieved or generated.
            identityPoolId: this.options.IdentityPoolId,
            clientConfig: { region: this.options.Region },
        })();
    }
}

function createClient(provider: AWSCognitoCredentialsProvider, handleMessage: (msg: any, topic: string) => void): mqtt5.Mqtt5Client {

    let wsConfig: iot.WebsocketSigv4Config = {
        credentialsProvider: provider,
        region: AWS_REGION
    }

    let builder: iot.AwsIotMqtt5ClientConfigBuilder = iot.AwsIotMqtt5ClientConfigBuilder.newWebsocketMqttBuilderWithSigv4Auth(
        AWS_IOT_ENDPOINT,
        wsConfig
    )

    let client: mqtt5.Mqtt5Client = new mqtt5.Mqtt5Client(builder.build());

    client.on('error', (error) => {
        console.log("Error event: " + error.toString());
    });

    client.on("messageReceived", (eventData: mqtt5.MessageReceivedEvent): void => {
        console.log("Message Received event: " + JSON.stringify(eventData.message));
        if (eventData.message.payload) {
            handleMessage(toUtf8(eventData.message.payload as Buffer), eventData.message.topicName)
        }
    });

    client.on('attemptingConnect', (eventData: mqtt5.AttemptingConnectEvent) => {
        console.log("Attempting Connect event");
    });

    client.on('connectionSuccess', (eventData: mqtt5.ConnectionSuccessEvent) => {
        console.log("Connection Success event");

    });

    client.on('connectionFailure', (eventData: mqtt5.ConnectionFailureEvent) => {
        console.log("Connection failure event: " + eventData.error.toString());
    });

    client.on('disconnection', (eventData: mqtt5.DisconnectionEvent) => {
        console.log("Disconnection event: " + eventData.error.toString());
        if (eventData.disconnect !== undefined) {
            console.log('Disconnect packet: ' + JSON.stringify(eventData.disconnect));
        }
    });

    client.on('stopped', (eventData: mqtt5.StoppedEvent) => {
        console.log("Stopped event");
    });

    return client;
}

export {createClient, AWSCognitoCredentialsProvider};