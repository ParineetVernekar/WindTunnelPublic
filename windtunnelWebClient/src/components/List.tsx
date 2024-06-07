// src/ItemList.tsx

import React from 'react';
import {  ListItem, UnorderedList, Box, Text, Card, CardBody, HStack } from '@chakra-ui/react'
import '../styles/ListStyles.css'
import { Icon } from '@chakra-ui/react'
import {  FaDroplet, FaTemperatureHalf, FaWeightHanging, FaFan, FaWind } from "react-icons/fa6";

interface ItemListProps {
    items: any[];
    thingName: string;
}

const StatusList: React.FC<ItemListProps> = ({ items }) => {
    return (
        <UnorderedList styleType={"none"}>
            {items.map((item, index) => (
                <ListItem key={index} className="statusListItem">
                    {item.content.message == "CONNECTED" && (
                        <Card variant={'elevated'}>
                            <CardBody>
                                <Box style={{ flexDirection: 'column', display: 'flex' }}>
                                    <Text fontWeight={'bold'}>{item.content.message}</Text>
                                    <Text fontSize={'xs'}>{item.content.device.name} connected at {item.content.device.ip}</Text>
                                    <Text fontSize={'xs'}>Running v{item.content.software.version} {item.content.software.flavour}</Text>

                                </Box>
                            </CardBody>
                        </Card>
                    )}
                    {item.content.message != "CONNECTED" && (
                        <Card>
                            <CardBody>
                                <Text fontWeight={'bold'}>{item.content.message}</Text>
                                <Text pt='1' fontSize={'xs'}>{item.content.device.name}</Text>

                            </CardBody>
                        </Card>
                    )}

                </ListItem>
            ))}
        </UnorderedList>
    );
};

const ItemList: React.FC<ItemListProps> = ({ items, thingName }) => {
    return (
        <UnorderedList styleType={"none"}>
            {
                thingName == 'ConditionThing' &&
                <Box>
                    {items.map((item, index) => (
                        <ListItem key={index} my='3'>
                            <Card>
                                <CardBody p='3' >
                                    <HStack >
                                        <Icon as={FaTemperatureHalf} />
                                        <Text>
                                            {item.content.value_temperature?.toFixed(2)}°C
                                        </Text>
                                    </HStack>
                                    <HStack>
                                        <Icon as={FaDroplet} />
                                        <Text>
                                            {item.content.value_humidity?.toFixed(0)}%

                                        </Text>
                                    </HStack>
                                    <HStack>
                                        <Icon as={FaWind} />
                                        <Text>
                                            {item.content.value_airspeed?.toFixed(2)}m/s

                                        </Text>
                                    </HStack>
                                    <Text pt='3' fontSize={'xs'}>{item.content.run_id} - {new Date(Number(item.content.timestamp) * 1000).toLocaleTimeString()}</Text>
                                </CardBody>

                            </Card>
                        </ListItem>
                    ))}
                </Box>
            }


            {
                ((thingName == 'DragThing') || (thingName == 'DownforceThing')) &&
                <Box>
                    {items.map((item, index) => (
                        <ListItem key={index} my='3'>
                            <Card>
                                <CardBody p='3' >
                                    <HStack >
                                        <Icon as={FaWeightHanging} />
                                        <Text>
                                            {(Math.abs(item.content.value?.toFixed(5))* 0.009807).toFixed(4)}N
                                        </Text>
                                    </HStack>
                                    <Text pt='3' fontSize={'xs'}>{item.content.run_id} - {new Date(Number(item.content.timestamp) * 1000).toLocaleTimeString()}</Text>
                                </CardBody>

                            </Card>
                        </ListItem>
                    ))}
                </Box>
            }

            {
                (thingName == 'FanThing') &&
                <Box>
                    {items.map((item, index) => (
                        <ListItem key={index} my='3'>
                            <Card>
                                <CardBody p='3' >
                                    <HStack >
                                        <Icon as={FaFan} />
                                        <Text>
                                            {item.content.value?.toFixed(2)}V
                                        </Text>
                                    </HStack>
                                    <Text pt='3' fontSize={'xs'}>{item.content.run_id} - {new Date(Number(item.content.timestamp) * 1000).toLocaleTimeString()}</Text>
                                </CardBody>

                            </Card>
                        </ListItem>
                    ))}
                </Box>
            }
        </UnorderedList>
    );
};

export { ItemList, StatusList };
