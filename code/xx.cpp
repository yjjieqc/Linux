struct FlowEntry{
    u64     key;
    u32     src_ip;
    u32     dst_ip;
    u16     src_port;
    u16     dst_port;
    u8      priority;
    u32     flowsize;
    ...
}