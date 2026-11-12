/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Config.hpp>
#include <Prism/Core/Platform.hpp>
#include <Prism/Core/LanguageSupport.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/Core/Error.hpp>
#include <Prism/Debug/Assertions.hpp>
#include <Prism/Debug/Log.hpp>

export module Prism;

// Every Prism module. The macros still come from the headers above.
export import Prism.Algorithm.FNV1aHash;
export import Prism.Algorithm.Find;
export import Prism.Algorithm.Hash;
export import Prism.Algorithm.Random;
export import Prism.Algorithm.SearchString;
export import Prism.Algorithm.Sort;
export import Prism.Containers.Array;
export import Prism.Containers.BitSpan;
export import Prism.Containers.Bitmap;
export import Prism.Containers.CircularQueue;
export import Prism.Containers.Deque;
export import Prism.Containers.DoublyLinkedList;
export import Prism.Containers.IntervalTree;
export import Prism.Containers.IntrusiveList;
export import Prism.Containers.IntrusiveListV3;
export import Prism.Containers.IntrusiveRedBlackTree;
export import Prism.Containers.IntrusiveRefList;
export import Prism.Containers.KeyValuePair;
export import Prism.Containers.Queue;
export import Prism.Containers.RadixTree;
export import Prism.Containers.RedBlackTree;
export import Prism.Containers.RingBuffer;
export import Prism.Containers.Set;
export import Prism.Containers.Span;
export import Prism.Containers.SplitBuffer;
export import Prism.Containers.Stack;
export import Prism.Containers.Tuple;
export import Prism.Containers.UnorderedMap;
export import Prism.Containers.Vector;
export import Prism.Core.AllocatorTraits;
export import Prism.Core.Bits;
export import Prism.Core.Concepts;
export import Prism.Core.Core;
export import Prism.Core.Error;
export import Prism.Core.Eternal;
export import Prism.Core.HashTraits;
export import Prism.Core.Integer128;
export import Prism.Core.Iterator;
export import Prism.Core.Limits;
export import Prism.Core.NonCopyable;
export import Prism.Core.NonMovable;
export import Prism.Core.ParameterPack;
export import Prism.Core.Ranges;
export import Prism.Core.Singleton;
export import Prism.Core.TypeTraits;
export import Prism.Core.TypeTraitsForward;
export import Prism.Core.UUID;
export import Prism.Debug.Assertions;
export import Prism.Debug.Log;
export import Prism.Debug.LogSink;
export import Prism.Debug.Logger;
export import Prism.Debug.SourceLocation;
export import Prism.Debug.Stacktrace;
export import Prism.Math.Matrix;
export import Prism.Math.SimdVector;
export import Prism.Math.Vector;
export import Prism.Memory.Allocator;
export import Prism.Memory.BitStream;
export import Prism.Memory.Buffer;
export import Prism.Memory.BumpAllocator;
export import Prism.Memory.ByteStream;
export import Prism.Memory.Endian;
export import Prism.Memory.Memory;
export import Prism.Memory.Pointer;
export import Prism.Memory.Ref;
export import Prism.Memory.RefCounted;
export import Prism.Memory.SSOStorage;
export import Prism.Memory.Scope;
export import Prism.Memory.SlabAllocator;
export import Prism.Memory.SlabPool;
export import Prism.Memory.SlobAllocator;
export import Prism.Memory.WeakRef;
export import Prism.Network.IPv4Address;
export import Prism.Ranges.Range;
export import Prism.Ranges.Ranges;
export import Prism.Ranges.Views;
export import Prism.String.CharTraits;
export import Prism.String.CodePoints;
export import Prism.String.FormatHandler;
export import Prism.String.FormatString;
export import Prism.String.Formatter;
export import Prism.String.FormatterContext;
export import Prism.String.Printf;
export import Prism.String.String;
export import Prism.String.StringBuilder;
export import Prism.String.StringUtils;
export import Prism.String.StringView;
export import Prism.Utility.Atomic;
export import Prism.Utility.AtomicBuiltins;
export import Prism.Utility.Checksum;
export import Prism.Utility.Compare;
export import Prism.Utility.Delegate;
export import Prism.Utility.Enum;
export import Prism.Utility.Expected;
export import Prism.Utility.Hash;
export import Prism.Utility.LockingPolicy;
export import Prism.Utility.Math;
export import Prism.Utility.Mutex;
export import Prism.Utility.OptionParser;
export import Prism.Utility.Optional;
export import Prism.Utility.Path;
export import Prism.Utility.PathView;
export import Prism.Utility.RangeAllocator;
export import Prism.Utility.SimdIntrinsics;
export import Prism.Utility.SimdVector;
export import Prism.Utility.Time;
