// Copyright 2023 Bloomberg Finance L.P.
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// bmqimp_queue.t.cpp                                                 -*-C++-*-
#include <bmqimp_queue.h>

// BMQ
#include <bmqimp_stat.h>
#include <bmqp_ctrlmsg_messages.h>
#include <bmqp_eventutil.h>
#include <bmqp_protocol.h>
#include <bmqp_queueid.h>
#include <bmqt_uri.h>

#include <bmqst_statcontext.h>
#include <bmqu_memoutstream.h>

// BDE
#include <bsl_ios.h>
#include <bslmf_assert.h>
#include <bsls_assert.h>

// TEST DRIVER
#include <bmqtst_testhelper.h>

// CONVENIENCE
using namespace BloombergLP;
using namespace bsl;

// ============================================================================
//                                    TESTS
// ----------------------------------------------------------------------------
namespace {

static void test1_breathingTest()
// --------------------------------------------------------------------
// BREATHING TEST
//
// Concerns:
//   Exercise the basic functionality of the component.
//
// Plan:
//   1) Assert consistency of publicly exposed constants
//   2) Exercise the basic functionality of the component
//
// Testing:
//   Basic functionality
// --------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("BREATHING TEST");

    // Constants
    const int k_INVALID_QUEUE_ID     = bmqimp::Queue::k_INVALID_QUEUE_ID;
    const int k_INVALID_CONFIGURE_ID = bmqimp::Queue::k_INVALID_CONFIGURE_ID;

    bmqimp::QueueState::Enum  k_STATE = bmqimp::QueueState::e_CLOSED;
    const bmqt::CorrelationId k_CORID;
    const unsigned int        k_SQID = 0U;
    bmqimp::Queue             obj(bmqtst::TestHelperUtil::allocator());
    bmqt::QueueOptions        options(bmqtst::TestHelperUtil::allocator());

    options.setMaxUnconfirmedMessages(0)
        .setMaxUnconfirmedBytes(0)
        .setConsumerPriority(bmqp::Protocol::k_CONSUMER_PRIORITY_INVALID);

    // Verify default state
    BMQTST_ASSERT_EQ(obj.isValid(), false);
    BMQTST_ASSERT_EQ(obj.id(), k_INVALID_QUEUE_ID);
    BMQTST_ASSERT_EQ(obj.pendingConfigureId(), k_INVALID_CONFIGURE_ID);
    BMQTST_ASSERT_EQ(obj.flags(), 0u);

    BMQTST_ASSERT(obj.hasDefaultSubQueueId());

    BMQTST_ASSERT_EQ(obj.uri(), "");
    BMQTST_ASSERT_EQ(obj.state(), k_STATE);
    BMQTST_ASSERT_EQ(obj.subQueueId(), k_SQID);
    BMQTST_ASSERT_EQ(obj.correlationId(), k_CORID);
    BMQTST_ASSERT_EQ(obj.options(), options);
    BMQTST_ASSERT_EQ(obj.atMostOnce(), false);

    BMQTST_ASSERT_EQ(obj.handleParameters().qId(),
                     static_cast<unsigned int>(k_INVALID_QUEUE_ID));

    BMQTST_ASSERT_EQ(obj.hasMultipleSubStreams(), false);
    BMQTST_ASSERT_EQ(obj.handleParameters().uri(), "")
    BMQTST_ASSERT_EQ(obj.handleParameters().writeCount(), 0);
    BMQTST_ASSERT_EQ(obj.handleParameters().readCount(), 0);
    BMQTST_ASSERT_EQ(obj.handleParameters().adminCount(), 0);
}

static void test2_settersTest()
// --------------------------------------------------------------------
// SETTERS TEST
//
// Concerns:
//   Exercise setters and getters functionality of the component.
//
// Plan:
//   1) Create bmqimp::Queue object and using it's setters set predefined
//      values
//   2) Call object getters and compare returned values with the predefined
//      ones
//
// Testing:
//   Setters and getters
// --------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("SETTERS TEST");

    bmqimp::Queue obj(bmqtst::TestHelperUtil::allocator());

    // Check setters
    const char k_URI[] = "bmq://ts.trades.myapp/my.queue?id=my.app";

    bmqt::Uri uri(k_URI, bmqtst::TestHelperUtil::allocator());

    const unsigned int        k_SQID       = 2U;
    const unsigned int        k_ID         = 12345;
    const int                 k_PENDING_ID = 65432;
    bmqimp::QueueState::Enum  k_STATE      = bmqimp::QueueState::e_OPENED;
    const bmqt::CorrelationId k_CORID      = bmqt::CorrelationId::autoValue();

    bsls::Types::Uint64 flags = 0;
    bmqt::QueueFlagsUtil::setReader(&flags);
    bmqt::QueueFlagsUtil::setWriter(&flags);
    bmqt::QueueFlagsUtil::setAdmin(&flags);

    bmqt::QueueOptions options(bmqtst::TestHelperUtil::allocator());
    options.setMaxUnconfirmedBytes(123);

    obj.setUri(uri)
        .setSubQueueId(k_SQID)
        .setState(k_STATE)
        .setId(k_ID)
        .setCorrelationId(k_CORID)
        .setFlags(flags)
        .setAtMostOnce(true)
        .setHasMultipleSubStreams(true)
        .setOptions(options)
        .setPendingConfigureId(k_PENDING_ID);

    BMQTST_ASSERT_EQ(obj.uri(), uri);
    BMQTST_ASSERT_EQ(obj.state(), k_STATE);
    BMQTST_ASSERT_EQ(obj.subQueueId(), k_SQID);
    BMQTST_ASSERT_EQ(obj.correlationId(), k_CORID);
    BMQTST_ASSERT_EQ(obj.flags(), flags);
    BMQTST_ASSERT_EQ(obj.options(), options);
    BMQTST_ASSERT_EQ(obj.isValid(), true);
    BMQTST_ASSERT_EQ(obj.atMostOnce(), true);
    BMQTST_ASSERT_EQ(obj.id(), static_cast<int>(k_ID));

    BMQTST_ASSERT_EQ(obj.hasMultipleSubStreams(), true);
    BMQTST_ASSERT_EQ(obj.pendingConfigureId(), k_PENDING_ID);
    BMQTST_ASSERT_EQ(obj.handleParameters().qId(), k_ID);
    BMQTST_ASSERT_EQ(obj.handleParameters().uri(), uri.asString())
    BMQTST_ASSERT_EQ(obj.handleParameters().writeCount(), 1);
    BMQTST_ASSERT_EQ(obj.handleParameters().readCount(), 1);
    BMQTST_ASSERT_EQ(obj.handleParameters().adminCount(), 1);

    BMQTST_ASSERT(!obj.hasDefaultSubQueueId());
}

static void test3_printQueueStateTest()
// --------------------------------------------------------------------
// PRINT QUEUE STATE TEST
//
// Concerns:
//   Validate output of bmqimp::QueueState::print() and << operator.
//
// Plan:
//   For each bmqimp::QueueState::Enum call 'print' and '<<' and check
//   the output with the expected pattern.
//
// Testing:
//   static
//   bsl::ostream&
//   bmqimp::QueueState::print(bsl::ostream&    stream,
//                             QueueState::Enum value,
//                             int              level          = 0,
//                             int              spacesPerLevel = 4);
//
//   bsl::ostream& operator<<(bsl::ostream&            stream,
//                            bmqimp::QueueState::Enum value);
// --------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("PRINT QUEUE STATE");

    PV("Testing print");

    BSLMF_ASSERT(bmqimp::QueueState::e_OPENING_OPN ==
                 bmqimp::QueueState::k_LOWEST_SUPPORTED_QUEUE_STATE);

    BSLMF_ASSERT(bmqimp::QueueState::e_CLOSING_CLS_EXPIRED ==
                 bmqimp::QueueState::k_HIGHEST_SUPPORTED_QUEUE_STATE);

    struct Test {
        bmqimp::QueueState::Enum d_type;
        const char*              d_expected;
    } k_DATA[] = {
        {bmqimp::QueueState::e_OPENING_OPN, "OPENING_OPN"},
        {bmqimp::QueueState::e_OPENING_CFG, "OPENING_CFG"},
        {bmqimp::QueueState::e_REOPENING_OPN, "REOPENING_OPN"},
        {bmqimp::QueueState::e_REOPENING_CFG, "REOPENING_CFG"},
        {bmqimp::QueueState::e_OPENED, "OPENED"},
        {bmqimp::QueueState::e_CLOSING_CFG, "CLOSING_CFG"},
        {bmqimp::QueueState::e_CLOSING_CLS, "CLOSING_CLS"},
        {bmqimp::QueueState::e_CLOSED, "CLOSED"},
        {bmqimp::QueueState::e_PENDING, "PENDING"},
        {bmqimp::QueueState::e_OPENING_OPN_EXPIRED, "OPENING_OPN_EXPIRED"},
        {bmqimp::QueueState::e_OPENING_CFG_EXPIRED, "OPENING_CFG_EXPIRED"},
        {bmqimp::QueueState::e_CLOSING_CFG_EXPIRED, "CLOSING_CFG_EXPIRED"},
        {bmqimp::QueueState::e_CLOSING_CLS_EXPIRED, "CLOSING_CLS_EXPIRED"},
        {static_cast<bmqimp::QueueState::Enum>(-1), "(* UNKNOWN *)"}};

    const size_t k_NUM_DATA = sizeof(k_DATA) / sizeof(*k_DATA);

    for (size_t idx = 0; idx < k_NUM_DATA; ++idx) {
        const Test&        test = k_DATA[idx];
        bmqu::MemOutStream out(bmqtst::TestHelperUtil::allocator());
        bmqu::MemOutStream expected(bmqtst::TestHelperUtil::allocator());

        expected << test.d_expected << "\n";

        out.setstate(bsl::ios_base::badbit);
        bmqimp::QueueState::print(out, test.d_type, 0, 0);

        BMQTST_ASSERT_EQ(out.str(), "");

        out.clear();
        bmqimp::QueueState::print(out, test.d_type, 0, 0);

        BMQTST_ASSERT_EQ(out.str(), expected.str());

        out.reset();
        out << test.d_type;

        expected.reset();
        expected << test.d_expected;

        BMQTST_ASSERT_EQ(out.str(), expected.str());
    }
}

static void test4_printTest()
// --------------------------------------------------------------------
// PRINT TEST
//
// Concerns:
//   Validate output of bmqimp::Queue::print() and << operator.
//
// Plan:
//   For bmqimp::Queue object call 'print' and '<<' and check
//   the output with the expected pattern.
//
// Testing:
//   static
//   bsl::ostream&
//   bmqimp::Queue::print(bsl::ostream&    stream,
//                        int              level          = 0,
//                        int              spacesPerLevel = 4) const;
//
//   bsl::ostream& operator<<(bsl::ostream& stream,
//                            bmqimp::Queue rhs);
// --------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("PRINT");

    PV("Testing bmqimp::Queue print");

    const char k_URI[] = "bmq://ts.trades.myapp/my.queue?id=my.app";

    bmqt::Uri uri(k_URI, bmqtst::TestHelperUtil::allocator());

    const unsigned int        k_SQID       = 2U;
    const unsigned int        k_ID         = 12345;
    const int                 k_PENDING_ID = 65432;
    const int                 k_GROUP_ID   = 4091;
    bmqimp::QueueState::Enum  k_STATE      = bmqimp::QueueState::e_OPENED;
    const bmqt::CorrelationId k_CORID      = bmqt::CorrelationId(1);

    bsls::Types::Uint64 flags = 0;
    bmqt::QueueFlagsUtil::setReader(&flags);
    bmqt::QueueFlagsUtil::setWriter(&flags);

    bmqt::QueueOptions options(bmqtst::TestHelperUtil::allocator());
    options.setMaxUnconfirmedBytes(123);
    options.setMaxUnconfirmedMessages(5);
    options.setConsumerPriority(3);

    bmqimp::Queue obj(bmqtst::TestHelperUtil::allocator());
    obj.setUri(uri)
        .setSubQueueId(k_SQID)
        .setState(k_STATE)
        .setId(k_ID)
        .setCorrelationId(k_CORID)
        .setFlags(flags)
        .setAtMostOnce(true)
        .setHasMultipleSubStreams(true)
        .setOptions(options)
        .setPendingConfigureId(k_PENDING_ID)
        .setRequestGroupId(k_GROUP_ID);

    const char* k_PATTERN =
        "[ uri = bmq://ts.trades.myapp/my.queue?id=my.app "
        "flags = \"READ,WRITE\" atMostOnce = true hasMultipleSubStreams "
        "= true id = 12345 subQueueId = 2 appId = \"my.app\" correlationId = "
        "[ numeric = 1 ] state = OPENED options = [ "
        "maxUnconfirmedMessages = 5 maxUnconfirmedBytes = 123 "
        "consumerPriority = 3 suspendsOnBadHostHealth = false ] "
        "pendingConfigureId = 65432 requestGroupId = 4091 isSuspended = false "
        "isSuspendedWithBroker = false ]";

    bmqu::MemOutStream out(bmqtst::TestHelperUtil::allocator());
    bmqu::MemOutStream expected(bmqtst::TestHelperUtil::allocator());

    expected << k_PATTERN;

    out.setstate(bsl::ios_base::badbit);
    obj.print(out, 0, -1);

    BMQTST_ASSERT_EQ(out.str(), "");

    out.clear();
    obj.print(out, 0, -1);

    BMQTST_ASSERT_EQ(out.str(), expected.str());

    out.reset();
    out << obj;

    BMQTST_ASSERT_EQ(out.str(), expected.str());
}

static void test5_comparisionTest()
// --------------------------------------------------------------------
// COMPARISION TEST
//
// Concerns:
//   Exercise bmqimp::Queue comparison
//
// Plan:
//   1) Create two bmqimp::Queue object, for one of them set predefined
//      values using it's manipulators.
//   2) For the second object set the same values only for those fields
//      that are involved in comparison.
//   3) Verify that the objects are equal.
//
// Testing:
//   bool operator==(const bmqimp::Queue& lhs, const bmqimp::Queue& rhs);
// --------------------------------------------------------------------
{
    bmqtst::TestHelper::printTestName("COMPARISION TEST");

    bmqimp::Queue obj1(bmqtst::TestHelperUtil::allocator());
    bmqimp::Queue obj2(bmqtst::TestHelperUtil::allocator());

    BMQTST_ASSERT(obj1 == obj2);

    // Check setters
    const char k_URI[] = "bmq://ts.trades.myapp/my.queue?id=my.app";

    bmqt::Uri uri(k_URI, bmqtst::TestHelperUtil::allocator());

    const unsigned int        k_SQID       = 2U;
    const unsigned int        k_ID         = 12345;
    const int                 k_PENDING_ID = 65432;
    bmqimp::QueueState::Enum  k_STATE      = bmqimp::QueueState::e_OPENED;
    const bmqt::CorrelationId k_CORID      = bmqt::CorrelationId::autoValue();

    bsls::Types::Uint64 flags = 0;
    bmqt::QueueFlagsUtil::setReader(&flags);

    bmqt::QueueOptions options(bmqtst::TestHelperUtil::allocator());
    options.setMaxUnconfirmedBytes(123);

    obj1.setUri(uri)
        .setSubQueueId(k_SQID)
        .setState(k_STATE)
        .setId(k_ID)
        .setCorrelationId(k_CORID)
        .setFlags(flags)
        .setAtMostOnce(true)
        .setHasMultipleSubStreams(true)
        .setOptions(options)
        .setPendingConfigureId(k_PENDING_ID);

    BMQTST_ASSERT(obj1 != obj2);

    obj2.setUri(uri).setState(k_STATE).setCorrelationId(k_CORID).setFlags(
        flags);

    BMQTST_ASSERT(obj1 == obj2);
}

static void test6_statTest()
// --------------------------------------------------------------------
// STAT TEST
//
// Concerns:
//   Exercise the basic queue statistics initialization.
//
// Plan:
//   1) Create a valid bmqimp::Queue object and check that statistic
//      context can be initialized.
//
// Testing:
//   bmqimp::QueueStatsUtil and bmqimp::Queue statistic manipulators
// --------------------------------------------------------------------
{
    bmqtst::TestHelperUtil::ignoreCheckDefAlloc() = true;
    // Check for default allocator is explicitly disabled as
    // 'bmqst::TableSchema::addColumn' used in
    // 'bmqimp::QueueStatsUtil::initializeStats' may allocate
    // temporaries with default allocator.

    bmqtst::TestHelper::printTestName("STAT TEST");

    const char k_URI[] = "bmq://ts.trades.myapp/my.queue?id=my.app";

    bmqt::Uri                uri(k_URI, bmqtst::TestHelperUtil::allocator());
    bmqimp::QueueState::Enum k_STATE = bmqimp::QueueState::e_OPENED;
    bmqimp::Queue            obj(bmqtst::TestHelperUtil::allocator());

    bmqst::StatContextConfiguration config(
        "stats",
        bmqtst::TestHelperUtil::allocator());

    config.defaultHistorySize(1);

    bmqst::StatContext rootStatContext(config,
                                       bmqtst::TestHelperUtil::allocator());

    bmqst::StatValue::SnapshotLocation start;
    bmqst::StatValue::SnapshotLocation end;

    start.setLevel(0).setIndex(0);
    end.setLevel(0).setIndex(1);

    bmqimp::Stat queuesStats(bmqtst::TestHelperUtil::allocator());
    bmqimp::QueueStatsUtil::initializeStats(
        &queuesStats,
        &rootStatContext,
        start,
        end,
        bmqtst::TestHelperUtil::allocator());

    bmqst::StatContext* pStatContext = queuesStats.d_statContext_mp.get();

    BMQTST_ASSERT(pStatContext != 0);

    BMQTST_ASSERT_SAFE_FAIL(obj.registerStatContext(pStatContext));
    BMQTST_ASSERT_SAFE_FAIL(obj.statUpdateOnMessage(1, true));
    BMQTST_ASSERT_SAFE_FAIL(obj.statReportCompressionRatio(2));

    obj.setUri(uri);

    BMQTST_ASSERT_SAFE_FAIL(obj.registerStatContext(pStatContext));
    BMQTST_ASSERT_SAFE_FAIL(obj.statUpdateOnMessage(1, true));
    BMQTST_ASSERT_SAFE_FAIL(obj.statReportCompressionRatio(2));

    obj.setState(k_STATE);
    obj.registerStatContext(pStatContext);

    rootStatContext.snapshot();

    BMQTST_ASSERT_EQ(rootStatContext.numSubcontexts(), 1);

    const char                k_STAT_NAME[] = "queues";
    const bmqst::StatContext* k_pSubContext = rootStatContext.getSubcontext(
        k_STAT_NAME);

    BMQTST_ASSERT(k_pSubContext != 0);
    BMQTST_ASSERT_EQ(k_pSubContext->numValues(), 3);
    BMQTST_ASSERT_EQ(k_pSubContext->valueName(0), "in");
    BMQTST_ASSERT_EQ(k_pSubContext->valueName(1), "out");
    BMQTST_ASSERT_EQ(k_pSubContext->valueName(2), "compression_ratio");

    const bmqst::StatValue& k_IN_VALUE =
        k_pSubContext->value(bmqst::StatContext::e_TOTAL_VALUE, 0);

    const bmqst::StatValue& k_OUT_VALUE =
        k_pSubContext->value(bmqst::StatContext::e_TOTAL_VALUE, 1);

    const bmqst::StatValue& k_STAT_COMPRESSION_RATIO =
        k_pSubContext->value(bmqst::StatContext::e_TOTAL_VALUE, 2);

    const int k_NEW_OUT_VALUE = 1024;

    BMQTST_ASSERT_EQ(k_IN_VALUE.max(), 0);
    BMQTST_ASSERT_EQ(k_OUT_VALUE.max(), 0);
    BMQTST_ASSERT_EQ(k_STAT_COMPRESSION_RATIO.max(), 0);

    obj.statUpdateOnMessage(k_NEW_OUT_VALUE, true);
    obj.statReportCompressionRatio(2);
    rootStatContext.snapshot();

    BMQTST_ASSERT_EQ(k_IN_VALUE.max(), 0);
    BMQTST_ASSERT_EQ(k_OUT_VALUE.max(), k_NEW_OUT_VALUE);
    BMQTST_ASSERT_EQ(k_STAT_COMPRESSION_RATIO.max(),
                     2 * 10000);  // scaling factor

    obj.clearStatContext();
    BMQTST_ASSERT_SAFE_FAIL(obj.statUpdateOnMessage(1, true));
    BMQTST_ASSERT_SAFE_FAIL(obj.statReportCompressionRatio(2));
}

}  // close unnamed namespace

// ============================================================================
//                                 MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    TEST_PROLOG(bmqtst::TestHelper::e_DEFAULT);

    bmqt::UriParser::initialize(bmqtst::TestHelperUtil::allocator());

    switch (_testCase) {
    case 0:
    case 6: test6_statTest(); break;
    case 5: test5_comparisionTest(); break;
    case 4: test4_printTest(); break;
    case 3: test3_printQueueStateTest(); break;
    case 2: test2_settersTest(); break;
    case 1: test1_breathingTest(); break;
    default: {
        cerr << "WARNING: CASE '" << _testCase << "' NOT FOUND." << endl;
        bmqtst::TestHelperUtil::testStatus() = -1;
    } break;
    }

    bmqt::UriParser::shutdown();

    TEST_EPILOG(bmqtst::TestHelper::e_CHECK_DEF_GBL_ALLOC);
}
