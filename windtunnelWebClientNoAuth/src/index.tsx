/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */
import ReactDOM from 'react-dom/client';
import App from './App'
import { ColorModeScript } from '@chakra-ui/react'
import theme from './utilities/theme'

const root = ReactDOM.createRoot(
  document.getElementById('react_app') as HTMLElement
);
root.render(
  <>
  <ColorModeScript initialColorMode={theme.config.initialColorMode} />
  <App />
  </>

);
