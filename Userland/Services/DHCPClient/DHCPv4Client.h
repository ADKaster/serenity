/*
 * Copyright (c) 2020, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "DHCPv4.h"
#include <AK/HashMap.h>
#include <AK/OwnPtr.h>
#include <AK/Result.h>
#include <AK/String.h>
#include <AK/Vector.h>
#include <LibCore/UDPServer.h>
#include <net/if.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

struct InterfaceDescriptor {
    String m_ifname;
    MACAddress m_mac_address;
    IPv4Address m_current_ip_address { 0, 0, 0, 0 };
};

struct DHCPv4Transaction {
    DHCPv4Transaction(InterfaceDescriptor ifname)
        : interface(ifname)
    {
    }

    InterfaceDescriptor interface;
    bool accepted_offer { false };
    bool has_ip { false };
    u32 offered_lease_time { 0 };
};

class DHCPv4Client final : public Core::Object {
    C_OBJECT(DHCPv4Client)

public:
    explicit DHCPv4Client(Vector<InterfaceDescriptor> ifnames_for_immediate_discover, Vector<InterfaceDescriptor> ifnames_for_later);
    virtual ~DHCPv4Client() override;

    void dhcp_discover(const InterfaceDescriptor& ifname);
    void dhcp_request(DHCPv4Transaction& transaction, const DHCPv4Packet& packet);

    void process_incoming(const DHCPv4Packet& packet);

    bool id_is_registered(u32 id) { return m_ongoing_transactions.contains(id); }

    struct Interfaces {
        Vector<InterfaceDescriptor> ready;
        Vector<InterfaceDescriptor> not_ready;
    };
    static Result<Interfaces, String> get_discoverable_interfaces();

private:
    void try_discover_deferred_ifs();

    HashMap<u32, OwnPtr<DHCPv4Transaction>> m_ongoing_transactions;
    Vector<InterfaceDescriptor> m_ifnames_for_immediate_discover;
    Vector<InterfaceDescriptor> m_ifnames_for_later;
    RefPtr<Core::UDPServer> m_server;
    RefPtr<Core::Timer> m_fail_check_timer;
    int m_max_timer_backoff_interval { 600000 }; // 10 minutes

    void handle_offer(const DHCPv4Packet&, const ParsedDHCPv4Options&);
    void handle_ack(const DHCPv4Packet&, const ParsedDHCPv4Options&);
    void handle_nak(const DHCPv4Packet&, const ParsedDHCPv4Options&);
};
