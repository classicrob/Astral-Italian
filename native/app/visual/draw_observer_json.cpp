#include "draw_observer.hpp"
#include <charconv>
#include <ostream>
#include <system_error>

namespace PetariNative::Visual {
namespace {
void number(std::ostream& out,std::uint64_t value) {
    char bytes[32];const auto result=std::to_chars(bytes,bytes+sizeof(bytes),value);
    if (result.ec!=std::errc{}) { out.setstate(std::ios::failbit);return; }
    out.write(bytes,result.ptr-bytes);
}
void signedNumber(std::ostream& out,std::int32_t value) {
    char bytes[32];const auto result=std::to_chars(bytes,bytes+sizeof(bytes),value);
    if (result.ec!=std::errc{}) { out.setstate(std::ios::failbit);return; }
    out.write(bytes,result.ptr-bytes);
}
void text(std::ostream& out,const char* value) { while (*value) out.put(*value++); }
void field(std::ostream& out,const char* name,std::uint64_t value) {
    out.put(',');out.put('"');text(out,name);text(out,"\":");number(out,value);
}
void signedField(std::ostream& out,const char* name,std::int32_t value) {
    out.put(',');out.put('"');text(out,name);text(out,"\":");signedNumber(out,value);
}
void hexName(std::ostream& out,const char* name,const char (&value)[64]) {
    static constexpr char digits[]="0123456789abcdef";
    out.put(',');out.put('"');text(out,name);text(out,"\":\"");
    for (std::size_t i=0;i<64 && value[i];++i) {
        const auto byte=static_cast<unsigned char>(value[i]);out.put(digits[byte>>4]);out.put(digits[byte&15]);
    }
    out.put('"');
}
void key(std::ostream& out,const SceneKey& value) {
    text(out,"{\"key_version\":1");hexName(out,"scene_hex",value.scene);hexName(out,"stage_hex",value.stage);
    signedField(out,"placed_scenario",value.placedScenario);signedField(out,"selected_scenario",value.selectedScenario);out.put('}');
}
}
bool writeDrawObservations(std::ostream& out,const DrawObserver& observer) {
    if (observer.installed()) return false;
    try {
        text(out,"{\"schema_version\":1,\"status\":\"detached-observations-only\",\"qualification\":\"BLOCKED\",\"qualified_frames\":null,\"runtime_provider_accepted\":false");
        field(out,"fault",static_cast<std::uint8_t>(observer.fault()));field(out,"capacity",DrawEventCapacity);
        field(out,"event_count",observer.size());text(out,",\"events\":[");
        for (std::size_t i=0;i<observer.size();++i) {
            if (i) out.put(',');const auto& e=observer.data()[i];const auto& s=e.snapshot;
            text(out,"{\"event_serial\":");number(out,e.eventSerial);field(out,"loop_serial",e.loopSerial);field(out,"draw_serial",e.drawSerial);
            field(out,"phase",static_cast<std::uint8_t>(e.phase));field(out,"source_key_matches",e.sourceKeyMatches);
            text(out,",\"source\":{\"frame\":");number(out,e.source.sourceFrame);field(out,"sampled_epoch",e.source.sampledEpoch);field(out,"valid",e.source.valid);
            text(out,",\"authority\":\"caller-sampled-identity-not-lifecycle\",\"key\":");key(out,e.source.key);out.put('}');
            text(out,",\"snapshot\":{\"key\":");key(out,s.key);
            signedField(out,"init_state",s.initState);signedField(out,"buffer_mode",s.bufferMode);
            signedField(out,"render_interval",s.renderInterval);signedField(out,"render_counter",s.renderCounter);
            signedField(out,"drawing_index",s.drawingIndex);signedField(out,"drawn_index",s.drawnIndex);
            signedField(out,"displaying_index",s.displayingIndex);signedField(out,"single_drawing_flag",s.singleDrawingFlag);
            field(out,"controller_present",s.controllerPresent);field(out,"current_scene_nonnull",s.currentSceneNonnull);
            field(out,"executed_scene_nonnull",s.executedSceneNonnull);field(out,"executed_scene_is_current",s.executedSceneIsCurrent);
            field(out,"initialized_game",s.initializedGame);field(out,"canonical_player_valid",s.canonicalPlayerValid);
            field(out,"framework_present",s.frameworkPresent);field(out,"xfb_present",s.xfbPresent);
            field(out,"render_flag",s.renderFlag);field(out,"invalid",s.invalid);text(out,"}}");
        }
        text(out,"]}\n");return out.good();
    } catch (const std::ios_base::failure&) { return false; }
}
} // namespace PetariNative::Visual
