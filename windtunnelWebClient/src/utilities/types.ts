 import { WithAuthenticatorProps } from "@aws-amplify/ui-react";
 interface IoTMessage {
    type: string;
    content: any;
    topic: string;
}

interface Device {
    name: string;
    displayName: string;
    topicReceiveMessages: string;
    topicSendMessages: string;
}

interface SelectDeviceProps {
    options: any[];
    onOptionsSelect: (option?: Device) => void;
}

interface AppProps extends WithAuthenticatorProps {}

interface DialogProps {
    title:string;
    message: string;
    buttonTitle:string;
    buttonClicked:(...args: any[]) => any;
    
}

export {IoTMessage, Device, SelectDeviceProps, AppProps, DialogProps}