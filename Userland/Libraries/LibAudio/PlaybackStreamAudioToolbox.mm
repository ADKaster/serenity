/*
 * Copyright (c) 2023, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Error.h>
#include <AK/String.h>
#include <LibAudio/PlaybackStream.h>
#include <LibAudio/PlaybackStreamAudioToolbox.h>
#include <LibCore/ThreadedPromise.h>

// Several AK types conflict with MacOS types.
#define FixedPoint FixedPointMacOS
#define Duration DurationMacOS
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudioTypes/CoreAudioTypes.h>
#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#undef FixedPoint
#undef Duration

namespace Audio {

struct PlaybackStreamAudioToolbox::AudioToolboxState : public RefCounted<PlaybackStreamAudioToolbox::AudioToolboxState> {
    AudioQueueRef m_audio_queue { nullptr };
    dispatch_queue_t m_dispatch_queue { nullptr };
    RefPtr<PlaybackStreamAudioToolbox> m_stream { nullptr };
    AudioDataRequestCallback m_audio_data_request_callback;

    double m_current_sample_time { 0.0 };
    u32 m_sample_rate;

    void queue_did_request_data(AudioQueueRef, AudioQueueBufferRef)
    {
        // FIXME: actually push data into the buffer
    }
    ErrorOr<Duration> total_time_played();
};

ErrorOr<NonnullRefPtr<PlaybackStream>> PlaybackStreamAudioToolbox::create(OutputState initial_output_state, u32 sample_rate, u8 channels, u32 target_latency_ms, AudioDataRequestCallback&& data_request_callback)
{
    auto state = TRY(try_make_ref_counted<AudioToolboxState>());
    state->m_sample_rate = sample_rate;
    state->m_audio_data_request_callback = move(data_request_callback);

    auto dispatch_queue_name = TRY(String::formatted("Serenity.AudioQueue.{:p}\0", state.ptr()));
    state->m_dispatch_queue = dispatch_queue_create(dispatch_queue_name.bytes_as_string_view().characters_without_null_termination(), DISPATCH_QUEUE_SERIAL);

    // FIXME: lmao what are these supposed to be
    // mBitsPerChannel
    // mBytesPerFrame
    // mBytesPerPacket
    // mFramesPerPacket
    AudioStreamBasicDescription desc {};
    desc.mChannelsPerFrame = channels;
    desc.mFormatID = kAudioFormatLinearPCM;
    desc.mFormatFlags = kLinearPCMFormatFlagIsFloat;
    desc.mSampleRate = sample_rate;

    OSStatus ret = AudioQueueNewOutputWithDispatchQueue(&state->m_audio_queue, &desc, 0 /*reserved*/, state->m_dispatch_queue,
        ^(AudioQueueRef queue, AudioQueueBufferRef buffer) {
            state->queue_did_request_data(queue, buffer);
        });

    if (ret != 0)
        return Error::from_errno(ret);

    // FIXME: create AudioQueueBuffers
    (void)target_latency_ms;

    auto stream = TRY(adopt_nonnull_ref_or_enomem(new (nothrow) PlaybackStreamAudioToolbox(move(state))));

    if (initial_output_state == OutputState::Playing) {
        __block OSStatus dispatch_ret = 0;
        dispatch_sync(stream->m_impl->m_dispatch_queue, ^{
            dispatch_ret = AudioQueueStart(stream->m_impl->m_audio_queue, nullptr);
        });
        if (dispatch_ret != 0)
            return Error::from_errno(dispatch_ret);
    }
    return stream;
}

PlaybackStreamAudioToolbox::PlaybackStreamAudioToolbox(RefPtr<AudioToolboxState> impl)
    : m_impl(move(impl))
{
}

PlaybackStreamAudioToolbox::~PlaybackStreamAudioToolbox() = default;

void PlaybackStreamAudioToolbox::set_underrun_callback(Function<void()>)
{
}

NonnullRefPtr<Core::ThreadedPromise<Duration>> PlaybackStreamAudioToolbox::resume()
{
    auto promise = Core::ThreadedPromise<Duration>::create();
    dispatch_async(m_impl->m_dispatch_queue, ^{
        // FIXME: threading concerns? where does this run? are we sure the thing will be stopped when this is called?
        //        do we need to check an is_playing flag or w/e?
        AudioTimeStamp start_time = {
            .mSampleTime = m_impl->m_current_sample_time,
            .mFlags = kAudioTimeStampSampleTimeValid,
        };
        auto ret = AudioQueueStart(m_impl->m_audio_queue, &start_time);
        if (ret != 0) {
            promise->reject(Error::from_errno(ret));
            return;
        }
        auto maybe_duration = m_impl->total_time_played();
        if (maybe_duration.is_error())
            promise->reject(maybe_duration.release_error());
        else
            promise->resolve(maybe_duration.release_value());
    });
    return promise;
}

NonnullRefPtr<Core::ThreadedPromise<void>> PlaybackStreamAudioToolbox::drain_buffer_and_suspend()
{
    return Core::ThreadedPromise<void>::create();
}

NonnullRefPtr<Core::ThreadedPromise<void>> PlaybackStreamAudioToolbox::discard_buffer_and_suspend()
{
    return Core::ThreadedPromise<void>::create();
}

ErrorOr<Duration> PlaybackStreamAudioToolbox::AudioToolboxState::total_time_played()
{
    // NOTE: This must be run called from a dispatch queue!
    AudioTimeStamp out_time {};
    OSStatus ret = AudioQueueGetCurrentTime(m_audio_queue, nullptr, &out_time, nullptr);
    VERIFY(out_time.mFlags & kAudioTimeStampSampleTimeValid);
    if (ret != 0)
        return Error::from_errno(ret);
    // FIXME: overflow? etc
    return Duration::from_microseconds(static_cast<i64>(m_sample_rate * 1'000'00 * out_time.mSampleTime));
}

ErrorOr<Duration> PlaybackStreamAudioToolbox::total_time_played()
{
    ErrorOr<Duration> ret = Error::from_errno(0);

    struct Closure {
        PlaybackStreamAudioToolbox* that;
        ErrorOr<Duration>* output;
    } closure = { this, &ret };

    dispatch_sync_f(
        m_impl->m_dispatch_queue, &closure, +[](void* arg) {
            auto* ctx = static_cast<Closure*>(arg);
            *(ctx->output) = ctx->that->m_impl->total_time_played();
        });
    return ret;
}

NonnullRefPtr<Core::ThreadedPromise<void>> PlaybackStreamAudioToolbox::set_volume(double level)
{
    auto promise = Core::ThreadedPromise<void>::create();
    dispatch_async(m_impl->m_dispatch_queue, ^{
        OSStatus err = AudioQueueSetParameter(m_impl->m_audio_queue, kAudioQueueParam_Volume, static_cast<float>(level));
        if (err != 0)
            promise->reject(Error::from_errno(err));
        else
            promise->resolve();
    });
    return promise;
}

}
