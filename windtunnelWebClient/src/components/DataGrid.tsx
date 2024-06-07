import { Grid, GridItem, Box, Text, HStack } from "@chakra-ui/react"
import { StatusList, ItemList } from './List'
import { IoTMessage, Device } from "../utilities/types"
import { FaCircleCheck } from 'react-icons/fa6'
export const DataGrid: React.FC<{ messages: IoTMessage[], connectedDevices: Device[] }> = ({ messages, connectedDevices }) => {
    return (
        <Grid templateColumns='repeat(5, 1fr)' w="100%">
            <GridItem w="100%">
                <Box>
                    <Text textAlign={'center'}>Status</Text>
                    <StatusList items={messages.filter((message) => message.topic === 'windtunnel/status')} thingName='' />
                </Box>
            </GridItem>

            <GridItem w="100%">
                <Box>

                <HStack justifyContent={'center'}>
                    {connectedDevices.some(e => e.name === 'ConditionThing') &&
                        <FaCircleCheck fill='#007d4f' />
                    }
                    <Text textAlign={'center'}>Conditions</Text>

                    </HStack>

                    <ItemList items={messages.filter((message) => message.topic === 'windtunnel/ConditionThing/pub').slice(0, 10)} thingName='ConditionThing' />
                </Box>
            </GridItem>
            {/* <GridItem w="100%">
                <Box>
                <HStack justifyContent={'center'}>
                    {connectedDevices.some(e => e.name === 'DragThing') &&
                        <FaCircleCheck fill='#007d4f' />
                    }
                    <Text textAlign={'center'}>Drag</Text>

                    </HStack>

                    <ItemList items={messages.filter((message) => message.topic === 'windtunnel/DragThing/pub')} thingName='DragThing' />
                </Box>
            </GridItem> */}
            <GridItem w="100%">
                <Box>
                <HStack justifyContent={'center'}>
                    {connectedDevices.some(e => e.name === 'DownforceThing') &&
                        <FaCircleCheck fill='#007d4f' />
                    }
                    <Text textAlign={'center'}>Downforce</Text>

                    </HStack>
                    <ItemList items={messages.filter((message) => message.topic === 'windtunnel/DownforceThing/pub')} thingName='DownforceThing' />
                </Box>
            </GridItem>
            <GridItem w="100%">
                <Box>
                    <HStack justifyContent={'center'}>
                    {connectedDevices.some(e => e.name === 'FanThing') &&
                        <FaCircleCheck fill='#007d4f' />
                    }
                    <Text textAlign={'center'}>Fan</Text>

                    </HStack>
                    <ItemList items={messages.filter((message) => message.topic === 'windtunnel/FanThing/pub')} thingName='FanThing' />
                </Box>
            </GridItem>
        </Grid>
    )
}

