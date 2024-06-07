
import { useEffect } from 'react';
import { mqtt5 } from "aws-iot-device-sdk-v2";
import { once } from "events";
import { useState, useRef } from 'react';
import { AWS_REGION, AWS_COGNITO_IDENTITY_POOL_ID } from './utilities/settings';
import { ItemList, StatusList } from './components/List'
import {
    ChakraProvider,
    Button,
    Box,
    Text,
    Grid,
    GridItem,
    Input,
    Heading,
    Divider,
    Select,
    AlertDialog,
    AlertDialogBody,
    AlertDialogOverlay,
    AlertDialogFooter,
    AlertDialogContent,
    Tabs,
    TabList,
    Tab,
    TabPanel,
    TabPanels,
    VStack,
    HStack,
    Popover,
    PopoverContent,
    PopoverTrigger,
    useDisclosure,
    PopoverBody,
    PopoverCloseButton,
    PopoverHeader,
    PopoverFooter,
    AlertDialogHeader,
    AlertDialogCloseButton,
    NumberInput,
    NumberInputField,
    NumberInputStepper,
    NumberIncrementStepper,
    NumberDecrementStepper,
    Fade,
    Card,
    useColorMode
} from '@chakra-ui/react'
import { createClient, AWSCognitoCredentialsProvider } from './utilities/IoT';
import { devices } from './utilities/data'
import { DataGrid } from './components/DataGrid';
import SelectDevices from './components/SelectDevices';
import { IoTMessage, Device, AppProps, DialogProps } from './utilities/types';

import { FaPause, FaStop, FaPlay, FaPowerOff, FaTrash, FaChartLine, FaDatabase, FaArrowRightFromBracket, FaRegFaceMehBlank } from 'react-icons/fa6';
import theme from './utilities/theme'

const delay = (ms: number) => new Promise(res => setTimeout(res, ms));

function Mqtt5({ signOut, user }: AppProps) {
    // const { colorMode, toggleColorMode } = useColorMode()

    const clientRef = useRef<mqtt5.Mqtt5Client | null>(null);
    const [selectedDevice, setSelectedDevice] = useState<Device>();
    const [connectedDevices, setConnectedDevices] = useState<Device[]>([]);
    const [runId, setRunId] = useState('');
    const [enteredFanSpeed, setFanSpeed] = useState(0);
    const [currentRunStatus, setCurrentRunStatus] = useState('STOP');
    const [changingConnection, setChangingConnection] = useState(false);
    const [connected, setConnected] = useState('CONNECTED');
    const [messages, setMessages] = useState<IoTMessage[]>([]);
    const [currentDialog, setCurrentDialog] = useState<DialogProps>();
    const { isOpen, onOpen, onClose } = useDisclosure();
    const cancelRef = useRef(null);

    useEffect(() => {
        StartConnection();
        console.log(user)
    }, []);

    useEffect(() => {
        console.log(connectedDevices)
    }, [connectedDevices]);

    const handleMessage = (msg: any, topic: string) => {
        const msgJson = JSON.parse(msg);

console.log(msgJson)
        const message: IoTMessage = {
            type: msgJson.type,
            content: msgJson.content,
            topic: topic
        }

        if (message.content.message == 'CONNECTED') {
            const deviceName = msgJson.content.device.name
            var result = devices.find(obj => {
                return obj.name === deviceName
            }) as Device

            SendGoodMorning(result.topicSendMessages);

            setConnectedDevices(currentConnected => [...currentConnected, result])
        }

        setMessages(oldArray => [message, ...oldArray]);
    };

    const handleOptionSelect = (option?: Device) => {
        setSelectedDevice(option)
    }



    const disconnectButtonClicked = () => {
        setCurrentDialog(alerts[0])
        onOpen()
    }

    const handleSignOutClicked = () => {
        setCurrentDialog(alerts[1])
        onOpen()
    }

    async function StartConnection() {
        setChangingConnection(true);
        if (clientRef.current) {
            await CloseConnection();
        }
        const provider = new AWSCognitoCredentialsProvider({
            IdentityPoolId: AWS_COGNITO_IDENTITY_POOL_ID,
            Region: AWS_REGION
        });
        /** Make sure the credential provider fetched before setup the connection */
        await provider.refreshCredentials();

        const client = createClient(provider, handleMessage);
        clientRef.current = client;
        const attemptingConnect = once(client, "attemptingConnect");
        const connectionSuccess = once(client, "connectionSuccess");

        client.start();

        await attemptingConnect;
        await connectionSuccess;

        var subscriptions = []
        for (let i = 1; i < devices.length; i++) {
            subscriptions.push({ qos: mqtt5.QoS.AtLeastOnce, topicFilter: devices[i].topicReceiveMessages })
        }
        subscriptions.push({ qos: mqtt5.QoS.AtLeastOnce, topicFilter: 'windtunnel/status' })

        const suback = await client.subscribe({
            subscriptions: subscriptions
        });
        setConnected("CONNECTED");
        await delay(750);
        setChangingConnection(false);
    }

    async function PublishMessage(topic: string, message: string) {
        if (!clientRef.current) {
            console.log('Client is not connected');
            return;
        }
        const payload = JSON.stringify(message);
        try {
            const publishResult = await clientRef.current.publish({
                qos: mqtt5.QoS.AtLeastOnce,
                topicName: topic,
                payload: message
            });
        } catch (error) {
            console.log(`Error publishing: ${error}`);
        }

    }

    const SendGoodMorning = (topic: string) => {

        const timestamp = Math.floor(Date.now() / 1000)
        const message = {
            type: "STATUS",
            content: {
                message: "GOOD_MORNING",
                timestamp: timestamp.toString()
            }

        }
        PublishMessage(topic, JSON.stringify(message))
    }

    const StartRun = () => {
        const topicToSend = devices[0].topicSendMessages

        const message = {
            type: "ACTION",
            content: {
                message: "START",
                run_id: runId
            }

        }
        PublishMessage(topicToSend, JSON.stringify(message))


        setCurrentRunStatus('START')
    }

    const PauseRun = () => {
        const topicToSend = devices[0].topicSendMessages

        const message = {
            type: "ACTION",
            content: {
                message: "WAIT",
                run_id: runId
            }

        }
        PublishMessage(topicToSend!, JSON.stringify(message))

        setCurrentRunStatus('WAIT')
    }

    const StopRun = () => {
        const topicToSend = devices[0].topicSendMessages

        const message = {
            type: "ACTION",
            content: {
                message: "STOP"
            }

        }
        PublishMessage(topicToSend!, JSON.stringify(message))

        setCurrentRunStatus('STOP')
    }

    const CloseConnection = async () => {
        setChangingConnection(true);

        if (!clientRef.current) {
            console.log('Client is not connected');
            return;
        }
        const client = clientRef.current;

        const disconnection = once(client, "disconnection");
        const stopped = once(client, "stopped");

        client.stop();
        clientRef.current = null;
        onClose();

        await disconnection;
        await stopped;
        await delay(2000);
        setChangingConnection(false);
        setConnected("DISCONNECTED");

    }

    function ClearMessages() {
        setMessages([])
    }

    const handleFanSpeedChange = () =>{
        const topicToSend = devices[4].topicSendMessages

        //take new fan speed from state
console.log(enteredFanSpeed)
         // send wait message to public topic
         setRunId("TRANSITION");
         PauseRun();
        // send fan message to fan thing with new speed
        const message = {
            type: "ACTION",
            content: {
                message: "FAN_CHANGE",
                new_speed: enteredFanSpeed.toString()
            }
        }
        PublishMessage(topicToSend!, JSON.stringify(message))
       // wait for 5s

       delay(5000);
       // release UI so new run ID can be sent
    }

    const alerts: DialogProps[] = [
        {
            title: "Disconnect",
            message: "Are you sure you want to disconnect from the MQTT server?",
            buttonTitle: "Disconnect",
            buttonClicked: CloseConnection
        }, {
            title: "Sign out",
            message: "Are you sure you want to sign out?",
            buttonTitle: "Sign out",
            buttonClicked: signOut!
        }


    ]

    return (
        <>
                <Box p="15" h='100%'>
                    <Box flexDirection={'row'} display={'flex'} justifyContent={'space-between'}>
                        <Box>
                            <Heading>Wind tunnel</Heading>
                            {currentRunStatus != "STOP" &&
                                <Text>Current run id: {runId}</Text>
                            }
                            <Text>Current run status: {currentRunStatus}</Text>
                        </Box>
                        <Box flexDirection={'row'} display='flex'>
                            {connected == "CONNECTED" ?
                                <Box>

                                    <Button leftIcon={<FaTrash />} colorScheme='orange' mx='1.5' onClick={ClearMessages}>Clear messages</Button>
                                    <Button isLoading={changingConnection} leftIcon={<FaPowerOff />} colorScheme={'red'} mx='1.5' variant={'solid'} onClick={disconnectButtonClicked}>Disconnect</Button>
                                </Box>
                                :
                                <Box>
                                    <Button isLoading={changingConnection} leftIcon={<FaPowerOff />} colorScheme={'green'} variant={'solid'} onClick={() => StartConnection()}>Connect</Button>
                                </Box>
                            }
                            <Button leftIcon={<FaArrowRightFromBracket />} mx='1.5' onClick={handleSignOutClicked}><Text>Sign out</Text></Button>
                        </Box>
                    </Box>
                    <Divider my="4" />

                    <Box style={{ flexDirection: 'row', display: 'flex', height: '100%' }}>

                        <Tabs w='90%' variant={'enclosed'}>
                            <TabList>
                                <Tab><HStack><FaDatabase /><Text>Live Data</Text></HStack></Tab>
                                <Tab><HStack><FaChartLine /><Text>Graph Data</Text></HStack></Tab>
                            </TabList>

                            <TabPanels>
                                <TabPanel>
                                    <Box display='flex' justifyContent={'center'}>
                                        <DataGrid messages={messages} connectedDevices={connectedDevices} />
                                        {/* {messages.length == 0 ?
                                    <Card maxW={'300px'} minW='300px' display='flex' justifyContent={'center'}>
                                        <Box justifyContent={'center'}>
                                        <FaRegFaceMehBlank />
                                        </Box>
                                        <Text textAlign={'center'}> No messages</Text>
                                   
                                    </Card>
                                    :
                                    <DataGrid messages={messages} />
                                    } */}
                                    </Box>
                                </TabPanel>
                                <TabPanel>
                                    <Box>
                                        <Text>Graph data</Text>
                                    </Box>
                                </TabPanel>
                            </TabPanels>
                        </Tabs>

                        <Divider orientation='vertical' h='100%' ml='3' w='5' />

                        <Box w='30%'>



                            {((currentRunStatus == 'STOP') || (currentRunStatus == 'WAIT')) &&
                                <Box>
                                    <Input placeholder='Run ID' value={runId} onChange={(val) => setRunId(val.target.value)} />
                                    <Button isDisabled={runId == ''} leftIcon={<FaPlay />} mt='2' onClick={() => StartRun()}>Start run</Button>
                                </Box>
                            }
                            {currentRunStatus == 'START' &&
                                <Box>
                                    <Input placeholder='Run ID' value={runId} onChange={(val) => setRunId(val.target.value)} />
                                    <Button leftIcon={<FaPause />} mt='2' onClick={() => PauseRun()}>Pause run</Button>
                                </Box>
                            }

                            {((currentRunStatus == 'START') || (currentRunStatus == 'WAIT')) &&
                                <Box>
                                    <Popover>
                                        <PopoverTrigger>
                                            <Button leftIcon={<FaStop />} mt='4' colorScheme='red' variant={'solid'} >Stop run</Button>

                                        </PopoverTrigger>
                                        <PopoverContent>
                                            <PopoverCloseButton />
                                            <PopoverHeader fontWeight={'500'}>Stop Run</PopoverHeader>
                                            <PopoverBody>
                                                <Text>Are you sure you want to stop the run?</Text>
                                                <Button mt='3' colorScheme='red' variant={'solid'} onClick={() => StopRun()}>Stop run</Button>

                                            </PopoverBody>

                                        </PopoverContent>
                                    </Popover>
                                </Box>
                            }
                            <Divider my='3' />
                            <NumberInput defaultValue={0} min={0} max={255}  onChange={(val) => setFanSpeed(Number(val))}>
                                <NumberInputField />
                                <NumberInputStepper>
                                    <NumberIncrementStepper />
                                    <NumberDecrementStepper />
                                </NumberInputStepper>
                            </NumberInput>
                            <Button onClick={handleFanSpeedChange}>Change fan speed</Button>
                            <Divider my='3' />
                            <SelectDevices options={devices.slice(1)} onOptionsSelect={handleOptionSelect} />
                            <Button isDisabled={selectedDevice == undefined} mt='2' onClick={() => SendGoodMorning(selectedDevice!.topicSendMessages)}>Send good morning</Button>
                            {/* <Button onClick={toggleColorMode}>
        Toggle {colorMode === 'light' ? 'Dark' : 'Light'}
      </Button> */}
                        </Box>
                    </Box>
                </Box>

                <AlertDialog
                    isOpen={isOpen}
                    leastDestructiveRef={cancelRef}
                    onClose={onClose}
                >
                    <AlertDialogOverlay>
                        <AlertDialogContent>
                            <AlertDialogHeader>
                                {currentDialog?.title}
                            </AlertDialogHeader>
                            <AlertDialogCloseButton />
                            <AlertDialogBody>
                                {currentDialog?.message}
                            </AlertDialogBody>

                            <AlertDialogFooter>
                                <Button ref={cancelRef} onClick={onClose}>
                                    Cancel
                                </Button>
                                <Button colorScheme='red' onClick={currentDialog?.buttonClicked} ml={3}>
                                    {currentDialog?.buttonTitle}
                                </Button>
                            </AlertDialogFooter>
                        </AlertDialogContent>
                    </AlertDialogOverlay>
                </AlertDialog>
        </>

    );
}

export default Mqtt5;
