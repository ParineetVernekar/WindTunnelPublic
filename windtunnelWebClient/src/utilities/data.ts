 
const devices = [
    {
        name: "Public",
        displayName: "All devices",
        topicReceiveMessages: 'windtunnel/public',
        topicSendMessages: 'windtunnel/public'
    },
    {
        name: "ConditionThing",
        displayName: "Condition Device",
        topicReceiveMessages: 'windtunnel/ConditionThing/pub',
        topicSendMessages: 'windtunnel/ConditionThing/sub'
    },
    {
        name: "DragThing",
        displayName: "Drag Device",
        topicReceiveMessages: 'windtunnel/DragThing/pub',
        topicSendMessages: 'windtunnel/DragThing/sub'
    },
    {
        name: "DownforceThing",
        displayName: "Downforce Device",
        topicReceiveMessages: 'windtunnel/DownforceThing/pub',
        topicSendMessages: 'windtunnel/DownforceThing/sub'
    },
    {
        name: "FanThing",
        displayName: "Fan Device",
        topicReceiveMessages: 'windtunnel/FanThing/pub',
        topicSendMessages: 'windtunnel/FanThing/sub'
    },
]

export {devices};