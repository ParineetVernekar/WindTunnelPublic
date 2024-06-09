
import {SelectDeviceProps} from '../utilities/types'
import { Box, Select } from '@chakra-ui/react';

const SelectDevices: React.FC<SelectDeviceProps> = ({ options, onOptionsSelect }) => {

    const handleSelectChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
        console.log(event.target.value)
        if(event.target.value==""){
            onOptionsSelect(undefined)
        } else{
        onOptionsSelect(JSON.parse(event.target.value))
        }
    };

    return (
        <Box>
            <Select placeholder="Select device" onChange={handleSelectChange}>
                {options.map((option: any) => (
                    <option key={option.topicReceiveMessages} value={JSON.stringify(option)}>
                        {option.displayName}
                    </option>
                ))}
            </Select>
        </Box>
    );

}

export default SelectDevices;