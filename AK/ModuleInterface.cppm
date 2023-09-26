module;

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <new>
#include <malloc/malloc.h>

export module AK;

#include <AK/Time.h>
#include <AK/StringImpl.h>
#include <AK/SinglyLinkedListSizePolicy.h>
#include <AK/CharacterTypes.h>
#include <AK/Error.h>
#include <AK/BinarySearch.h>
#include <AK/URLParser.h>
#include <AK/MemoryStream.h>
#include <AK/FloatingPointStringConversions.h>
#include <AK/InsertionSort.h>
#include <AK/ScopedValueRollback.h>
#include <AK/OptionParser.h>
#include <AK/TypedTransfer.h>
#include <AK/AtomicRefCounted.h>
#include <AK/Traits.h>
#include <AK/IntrusiveList.h>
#include <AK/OwnPtr.h>
#include <AK/BuiltinWrappers.h>
#include <AK/JsonPath.h>
#include <AK/NonnullRefPtr.h>
#include <AK/BumpAllocator.h>
#include <AK/DateConstants.h>
#include <AK/IDAllocator.h>
#include <AK/Base64.h>
#include <AK/Demangle.h>
#include <AK/LEB128.h>
#include <AK/DoublyLinkedList.h>
#include <AK/GenericShorthands.h>
#include <AK/UUID.h>
#include <AK/AllOf.h>
#include <AK/Assertions.h>
#include <AK/FloatingPoint.h>
#include <AK/MaybeOwned.h>
#include <AK/ScopeGuard.h>
#include <AK/StringHash.h>
#include <AK/DOSPackedTime.h>
#include <AK/JsonObjectSerializer.h>
#include <AK/Types.h>
#include <AK/Singleton.h>
#include <AK/QuickSort.h>
#include <AK/HashTable.h>
#include <AK/CircularBuffer.h>
#include <AK/Endian.h>
#include <AK/Badge.h>
#include <AK/Find.h>
#include <AK/RecursionDecision.h>
#include <AK/StringUtils.h>
#include <AK/MACAddress.h>
#include <AK/IntegralMath.h>
#include <AK/ConstrainedStream.h>
#include <AK/GenericLexer.h>
#include <AK/HashFunctions.h>
#include <AK/Ptr32.h>
#include <AK/ScopeLogger.h>
#include <AK/kstdio.h>
#include <AK/StringFloatingPointConversions.h>
#include <AK/UnicodeUtils.h>
#include <AK/Result.h>
#include <AK/BigIntBase.h>
#include <AK/BinaryHeap.h>
#include <AK/UFixedBigInt.h>
#include <AK/ByteBuffer.h>
#include <AK/JsonParser.h>
#include <AK/StdLibExtraDetails.h>
#include <AK/CountingStream.h>
#include <AK/HashMap.h>
#include <AK/NeverDestroyed.h>
#include <AK/Stream.h>
#include <AK/MemMem.h>
#include <AK/FixedArray.h>
#include <AK/Weakable.h>
#include <AK/Userspace.h>
#include <AK/UBSanitizer.h>
#include <AK/Optional.h>
#include <AK/QuickSelect.h>
#include <AK/EnumBits.h>
#include <AK/IPv4Address.h>
#include <AK/DistinctNumeric.h>
#include <AK/DeprecatedFlyString.h>
#include <AK/SIMDExtras.h>
#include <AK/JsonValue.h>
#include <AK/kmalloc.h>
#include <AK/Utf16View.h>
#include <AK/Utf32View.h>
#include <AK/CheckedFormatString.h>
#include <AK/DeprecatedString.h>
#include <AK/NumericLimits.h>
#include <AK/IntrusiveDetails.h>
#include <AK/JsonObject.h>
#include <AK/BitStream.h>
#include <AK/DefaultDelete.h>
#include <AK/CircularDeque.h>
#include <AK/FixedStringBuffer.h>
#include <AK/RefCountForwarder.h>
#include <AK/Noncopyable.h>
#include <AK/URL.h>
#include <AK/StackInfo.h>
#include <AK/Hex.h>
#include <AK/IterationDecision.h>
#include <AK/SIMDMath.h>
#include <AK/NoAllocationGuard.h>
#include <AK/TypeList.h>
#include <AK/Statistics.h>
#include <AK/PublicMacros.h>
#include <AK/JsonArray.h>
#include <AK/Utf8View.h>
#include <AK/ArbitrarySizedEnum.h>
#include <AK/FlyString.h>
#include <AK/StringView.h>
#include <AK/Array.h>
#include <AK/SIMD.h>
#include <AK/Stack.h>
#include <AK/SinglyLinkedList.h>
#include <AK/JsonArraySerializer.h>
#include <AK/Checked.h>
#include <AK/Tuple.h>
#include <AK/Math.h>
#include <AK/Memory.h>
#include <AK/TemporaryChange.h>
#include <AK/UFixedBigIntDivision.h>
#include <AK/SourceLocation.h>
#include <AK/RefPtr.h>
#include <AK/Diagnostics.h>
#include <AK/Span.h>
#include <AK/WeakPtr.h>
#include <AK/DisjointChunks.h>
#include <AK/StringBuilder.h>
#include <AK/CircularQueue.h>
#include <AK/IPv6Address.h>
#include <AK/LexicalPath.h>
#include <AK/NonnullOwnPtr.h>
#include <AK/ExtraMathConstants.h>
#include <AK/SourceGenerator.h>
#include <AK/NumberFormat.h>
#include <AK/Format.h>
#include <AK/StdLibExtras.h>
#include <AK/IntrusiveListRelaxedConst.h>
#include <AK/RefCounted.h>
#include <AK/BinaryBufferWriter.h>
#include <AK/Vector.h>
#include <AK/Bitmap.h>
#include <AK/Trie.h>
#include <AK/AnyOf.h>
#include <AK/Concepts.h>
#include <AK/PrintfImplementation.h>
#include <AK/BitCast.h>
#include <AK/Platform.h>
#include <AK/Iterator.h>
#include <AK/Try.h>
#include <AK/Function.h>
#include <AK/FixedPoint.h>
#include <AK/TypeCasts.h>
#include <AK/Complex.h>
#include <AK/Queue.h>
#include <AK/Random.h>
#include <AK/ReverseIterator.h>
#include <AK/IntrusiveRedBlackTree.h>
#include <AK/FuzzyMatch.h>
#include <AK/RedBlackTree.h>
#include <AK/BitmapView.h>
#include <AK/Forward.h>
#include <AK/String.h>
#include <AK/Variant.h>
#include <AK/BufferedStream.h>
#include <AK/ByteReader.h>
#include <AK/Atomic.h>

#if ARCH(X86_64)
#include <AK/FPControl.h>
#endif
