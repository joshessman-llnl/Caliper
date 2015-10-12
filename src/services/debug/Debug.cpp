/// @file  Debug.cpp
/// @brief Caliper debug service

#include "../CaliperService.h"

#include <Caliper.h>
#include <Snapshot.h>

#include <csv/CsvSpec.h>
#include <Log.h>

#include <mutex>

using namespace cali;
using namespace std;

namespace
{

mutex dbg_mutex;

void create_attr_cb(Caliper* c, const Attribute& attr)
{
    lock_guard<mutex> lock(dbg_mutex);
    Log(2).stream() << "Event: create_attribute (attr = " << attr.name() << ")" << endl;
}

void begin_cb(Caliper* c, const Attribute& attr)
{
    lock_guard<mutex> lock(dbg_mutex);
    Log(2).stream() << "Event: pre_begin (attr = " << attr.name() << ")" << endl;
}

void end_cb(Caliper* c, const Attribute& attr)
{
    lock_guard<mutex> lock(dbg_mutex);
    Log(2).stream() << "Event: pre_end (attr = " << attr.name() << ")" << endl;
}

void set_cb(Caliper* c, const Attribute& attr)
{
    lock_guard<mutex> lock(dbg_mutex);
    Log(2).stream() << "Event: pre_set (attr = " << attr.name() << ")" << endl;
}

const char* scopestrings[] = { "", "process", "thread", "", "task" };

string scope2string(int scope)
{
    const cali_context_scope_t scopes[] = { 
        CALI_SCOPE_TASK, CALI_SCOPE_THREAD, CALI_SCOPE_PROCESS 
    };

    string out;

    for (cali_context_scope_t s : scopes)
        if (scope & s) {
            if (out.size())
                out.append(":");
            out.append(scopestrings[s]);
        }

    return out;
}

void create_context_cb(cali_context_scope_t scope, ContextBuffer* ctx)
{
    lock_guard<mutex> lock(dbg_mutex);
    Log(2).stream() << "Event: create_context (scope = " << scope2string(scope) << ")" << endl;
}

void destroy_context_cb(ContextBuffer* ctx)
{
    lock_guard<mutex> lock(dbg_mutex);
    Log(2).stream() << "Event: destroy_context" << endl;
}

void snapshot_cb(Caliper* c, int scope, const Caliper::Entry*, Snapshot*)
{
    lock_guard<mutex> lock(dbg_mutex);
    Log(2).stream() << "Event: snapshot (scope = " << scope2string(scope) << ")" << endl;
}

    void process_snapshot_cb(Caliper* c, const Caliper::Entry*, const Snapshot* sbuf)
{
    lock_guard<mutex> lock(dbg_mutex);

    auto write_rec_fn = [](const RecordDescriptor& rec, const int count[], const Variant* data[]) {
        CsvSpec::write_record(Log(2).stream() << "Event: process_snapshot: ", rec, count, data);
    };

    sbuf->push_record(write_rec_fn);
}

void finish_cb(Caliper* c)
{
    lock_guard<mutex> lock(dbg_mutex);
    Log(2).stream() << "Event: finish" << endl;
}

void debug_service_register(Caliper* c)
{
    c->events().create_attr_evt.connect(&create_attr_cb);
    c->events().pre_begin_evt.connect(&begin_cb);
    c->events().pre_end_evt.connect(&end_cb);
    c->events().pre_set_evt.connect(&set_cb);
    c->events().finish_evt.connect(&finish_cb);
    c->events().create_context_evt.connect(&create_context_cb);
    c->events().destroy_context_evt.connect(&destroy_context_cb);
    c->events().snapshot.connect(&snapshot_cb);
    c->events().process_snapshot.connect(&process_snapshot_cb);

    Log(1).stream() << "Registered debug service" << endl;
}

} // namespace

namespace cali
{
    CaliperService DebugService = { "debug", ::debug_service_register };
}
