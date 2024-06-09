# Wind Tunnel Web Client - No Auth

This web client provides access to AWS IoT via a friendly user interface. This version has no authentication; it doesn't require you to setup AWS users for anyone to login.


## To use

Navigate to 'src' then 'utilities', and open 'settings.ts', and replace the AWS region and IoT endpoint with your region and endpoint. Now, return to the AWS Console and search for AWS Cognito in the search bar. Create a user pool and an identity pool that uses this user pool. Go to IAM, and give the user pool access to AWS IoT. Then, copy the identity pool id from the console, and paste it to its appropriate place in the 'settings.ts' file, and save.

Open command prompt or terminal, and navigate to the WindTunnelWebClientNoAuth folder. Run ‘npm install’ and after it completes, run ‘npm start’. Open the URL outputted by the shell, and you should see your webclient!
