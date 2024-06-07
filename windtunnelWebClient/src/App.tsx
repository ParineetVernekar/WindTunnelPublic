
import { Amplify } from 'aws-amplify';
import { Authenticator} from '@aws-amplify/ui-react';
import awsconfig from './aws-exports';
import Mqtt5 from './PubSub5';
import '@aws-amplify/ui-react/styles.css';

import {Box, Image, Heading, ChakraProvider} from '@chakra-ui/react'
import { useTheme } from '@chakra-ui/react';
import theme from './utilities/theme'


Amplify.configure(awsconfig);

const components = {
  Header() {
    // const { tokens } = useTheme();

    return (
      <Box textAlign="center">
       <Heading>The Wind Tunnel</Heading>
      </Box>
    );
  }
}
export default function App() {
    return (
      <Authenticator hideSignUp={true} components={components}>
        {({ signOut, user }) => (
          <ChakraProvider theme={theme}>
          <Mqtt5 signOut={signOut} user={user} />
          </ChakraProvider>
        )}
      </Authenticator>
  );
}

