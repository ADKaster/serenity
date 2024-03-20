/*
 * Copyright (c) 2024, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Math.h>
#include <LibCrypto/Hash/HashManager.h>

namespace Crypto::Hash {

// https://www.rfc-editor.org/rfc/rfc3447#appendix-B.2.1
class MGF1 {
public:
    static ErrorOr<ByteBuffer> create_mask(HashKind hash_kind, ReadonlyBytes seed, size_t mask_len)
    {
        Manager manager;
        manager.initialize(hash_kind);

        auto h_len = manager.digest_size();

        // 1. If maskLen > 2^32 hLen, output "mask too long" and stop.
        if (mask_len > h_len << 32)
            return Error::from_string_literal("mask too long");

        // 2. Let T be the empty octet string.
        ByteBuffer t;

        // 3. For counter from 0 to \ceil (maskLen / hLen) - 1, do the following:
        for (size_t counter = 0; counter <= AK::ceil_div(mask_len, h_len) - 1; counter++) {
            // a. Convert counter to an octet string C of length 4 octets (see Section 4.1):
            //        C = I2OSP (counter, 4) .
            u8 c[4] = {};
            c[0] = (counter >> 24) & 0xff;
            c[1] = (counter >> 16) & 0xff;
            c[2] = (counter >> 8) & 0xff;
            c[3] = counter & 0xff;

            // b. Concatenate the hash of the seed mgfSeed and C to the octet string T:
            //        T = T || Hash(mgfSeed || C) .
            manager.update(seed);
            manager.update(ReadonlyBytes { c, 4 });
            t.append(manager.digest().immutable_data(), h_len);
        }

        // 4. Output the leading maskLen octets of T as the octet string mask.
        return t.slice(0, mask_len);
    }
};

}
