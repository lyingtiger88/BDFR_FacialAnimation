#include "bdfr/core/JsonCodec.h"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bdfr {

namespace {

struct JsonValue {
    enum class Type { Null, Number, String, Object, Array, Bool };
    Type type = Type::Null;
    double number = 0.0;
    std::string string;
    std::map<std::string, JsonValue> object;
    std::vector<JsonValue> array;
    bool boolean = false;
};

class Parser {
public:
    explicit Parser(const std::string& text) : text_(text) {}

    bool parse(JsonValue& value, std::string* error) {
        try {
            skipWs();
            value = parseValue();
            skipWs();
            if (pos_ != text_.size()) throw std::runtime_error("trailing JSON content");
            return true;
        } catch (const std::exception& ex) {
            if (error) *error = ex.what();
            return false;
        }
    }

private:
    JsonValue parseValue() {
        skipWs();
        if (pos_ >= text_.size()) throw std::runtime_error("unexpected end of JSON");
        const char c = text_[pos_];
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') {
            JsonValue v;
            v.type = JsonValue::Type::String;
            v.string = parseString();
            return v;
        }
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        return parseNumber();
    }

    JsonValue parseObject() {
        JsonValue v;
        v.type = JsonValue::Type::Object;
        expect('{');
        skipWs();
        if (peek('}')) { ++pos_; return v; }
        while (true) {
            skipWs();
            if (!peek('"')) throw std::runtime_error("expected object key");
            std::string key = parseString();
            skipWs();
            expect(':');
            v.object.emplace(std::move(key), parseValue());
            skipWs();
            if (peek('}')) { ++pos_; break; }
            expect(',');
        }
        return v;
    }

    JsonValue parseArray() {
        JsonValue v;
        v.type = JsonValue::Type::Array;
        expect('[');
        skipWs();
        if (peek(']')) { ++pos_; return v; }
        while (true) {
            v.array.push_back(parseValue());
            skipWs();
            if (peek(']')) { ++pos_; break; }
            expect(',');
        }
        return v;
    }

    JsonValue parseNumber() {
        const std::size_t start = pos_;
        if (peek('-')) ++pos_;
        while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
        if (peek('.')) {
            ++pos_;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
        }
        if (peek('e') || peek('E')) {
            ++pos_;
            if (peek('+') || peek('-')) ++pos_;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
        }
        if (start == pos_) throw std::runtime_error("expected number");
        JsonValue v;
        v.type = JsonValue::Type::Number;
        try {
            v.number = std::stod(text_.substr(start, pos_ - start));
        } catch (...) {
            throw std::runtime_error("invalid number");
        }
        if (!std::isfinite(v.number)) throw std::runtime_error("non-finite number not allowed");
        return v;
    }

    JsonValue parseBool() {
        JsonValue v;
        v.type = JsonValue::Type::Bool;
        if (text_.compare(pos_, 4, "true") == 0) {
            v.boolean = true;
            pos_ += 4;
        } else if (text_.compare(pos_, 5, "false") == 0) {
            v.boolean = false;
            pos_ += 5;
        } else {
            throw std::runtime_error("invalid boolean");
        }
        return v;
    }

    JsonValue parseNull() {
        if (text_.compare(pos_, 4, "null") != 0) throw std::runtime_error("invalid null");
        pos_ += 4;
        return {};
    }

    std::string parseString() {
        expect('"');
        std::string out;
        while (pos_ < text_.size()) {
            char c = text_[pos_++];
            if (c == '"') return out;
            if (c == '\\') {
                if (pos_ >= text_.size()) throw std::runtime_error("invalid string escape");
                const char e = text_[pos_++];
                switch (e) {
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    case '/': out.push_back('/'); break;
                    case 'b': out.push_back('\b'); break;
                    case 'f': out.push_back('\f'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    default: throw std::runtime_error("unsupported JSON escape");
                }
            } else {
                out.push_back(c);
            }
        }
        throw std::runtime_error("unterminated string");
    }

    void skipWs() {
        while (pos_ < text_.size() &&
               std::isspace(static_cast<unsigned char>(text_[pos_]))) ++pos_;
    }

    void expect(char c) {
        skipWs();
        if (pos_ >= text_.size() || text_[pos_] != c)
            throw std::runtime_error(std::string("expected '") + c + "'");
        ++pos_;
    }

    bool peek(char c) const {
        return pos_ < text_.size() && text_[pos_] == c;
    }

    const std::string& text_;
    std::size_t pos_ = 0;
};

std::string escapeJson(const std::string& s) {
    std::ostringstream out;
    for (char c : s) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: out << c; break;
        }
    }
    return out.str();
}

const JsonValue* member(const JsonValue& object, const std::string& key) {
    if (object.type != JsonValue::Type::Object) return nullptr;
    auto it = object.object.find(key);
    return it == object.object.end() ? nullptr : &it->second;
}

bool numberMember(const JsonValue& object, const std::string& key, double& out) {
    const JsonValue* v = member(object, key);
    if (!v || v->type != JsonValue::Type::Number) return false;
    out = v->number;
    return true;
}

bool stringMember(const JsonValue& object, const std::string& key, std::string& out) {
    const JsonValue* v = member(object, key);
    if (!v || v->type != JsonValue::Type::String) return false;
    out = v->string;
    return true;
}

std::string encodeFrameBody(const FacialFrame& frame) {
    std::ostringstream out;
    out << std::setprecision(12);
    out << "{\"schemaVersion\":" << frame.schemaVersion
        << ",\"timestamp\":" << frame.timestampSeconds
        << ",\"confidence\":" << frame.confidence
        << ",\"head\":{\"pitch\":" << frame.head.pitch
        << ",\"yaw\":" << frame.head.yaw
        << ",\"roll\":" << frame.head.roll << "}"
        << ",\"gaze\":{\"x\":" << frame.gaze.x
        << ",\"y\":" << frame.gaze.y
        << ",\"confidence\":" << frame.gaze.confidence << "}"
        << ",\"curves\":{";

    std::map<std::string, float> ordered(frame.curves.begin(), frame.curves.end());
    bool first = true;
    for (const auto& [id, value] : ordered) {
        if (!first) out << ',';
        first = false;
        out << "\"" << escapeJson(id) << "\":" << value;
    }
    out << "}}";
    return out.str();
}

bool decodeFrameValue(const JsonValue& root, FacialFrame& frame, std::string* error) {
    if (root.type != JsonValue::Type::Object) {
        if (error) *error = "frame root must be object";
        return false;
    }

    double schema = 0.0, timestamp = 0.0, confidence = 0.0;
    if (!numberMember(root, "schemaVersion", schema) ||
        !numberMember(root, "timestamp", timestamp) ||
        !numberMember(root, "confidence", confidence)) {
        if (error) *error = "frame missing required numeric fields";
        return false;
    }

    const JsonValue* head = member(root, "head");
    const JsonValue* gaze = member(root, "gaze");
    const JsonValue* curves = member(root, "curves");
    if (!head || !gaze || !curves || curves->type != JsonValue::Type::Object) {
        if (error) *error = "frame missing head/gaze/curves";
        return false;
    }

    double pitch=0, yaw=0, roll=0, gx=0, gy=0, gc=0;
    if (!numberMember(*head, "pitch", pitch) || !numberMember(*head, "yaw", yaw) ||
        !numberMember(*head, "roll", roll) || !numberMember(*gaze, "x", gx) ||
        !numberMember(*gaze, "y", gy) || !numberMember(*gaze, "confidence", gc)) {
        if (error) *error = "invalid head/gaze";
        return false;
    }

    FacialFrame out;
    out.schemaVersion = static_cast<std::uint32_t>(schema);
    out.timestampSeconds = timestamp;
    out.confidence = static_cast<float>(confidence);
    out.head = {static_cast<float>(pitch), static_cast<float>(yaw), static_cast<float>(roll)};
    out.gaze = {static_cast<float>(gx), static_cast<float>(gy), static_cast<float>(gc)};

    for (const auto& [id, value] : curves->object) {
        if (value.type != JsonValue::Type::Number) {
            if (error) *error = "curve value must be numeric";
            return false;
        }
        out.curves[id] = static_cast<float>(value.number);
    }

    frame = std::move(out);
    return true;
}


bool decodeSessionValue(const JsonValue& root, Session& session, std::string* error) {
    Session out;
    if (!stringMember(root, "id", out.id) ||
        !stringMember(root, "project", out.project) ||
        !stringMember(root, "scene", out.scene) ||
        !stringMember(root, "shot", out.shot)) {
        if (error) *error = "session missing string metadata";
        return false;
    }

    const JsonValue* takes = member(root, "takes");
    if (!takes || takes->type != JsonValue::Type::Array) {
        if (error) *error = "session takes array missing";
        return false;
    }

    for (const auto& tv : takes->array) {
        Take take;
        double duration=0, fpsNum=0, fpsDen=0;
        if (!stringMember(tv, "id", take.id) || !stringMember(tv, "name", take.name) ||
            !stringMember(tv, "actorId", take.actorId) || !stringMember(tv, "source", take.source) ||
            !numberMember(tv, "duration", duration) || !numberMember(tv, "fpsNum", fpsNum) ||
            !numberMember(tv, "fpsDen", fpsDen)) {
            if (error) *error = "invalid take";
            return false;
        }
        take.durationSeconds = duration;
        take.frameRateNumerator = static_cast<std::uint32_t>(fpsNum);
        take.frameRateDenominator = static_cast<std::uint32_t>(fpsDen);

        const JsonValue* ranges = member(tv, "dirtyRanges");
        if (!ranges || ranges->type != JsonValue::Type::Array) {
            if (error) *error = "take dirtyRanges missing";
            return false;
        }
        for (const auto& rv : ranges->array) {
            double start=0, end=0;
            if (!numberMember(rv, "start", start) || !numberMember(rv, "end", end)) {
                if (error) *error = "invalid dirty range";
                return false;
            }
            take.dirtyRanges.push_back({start, end});
        }
        if (!out.addTake(std::move(take))) {
            if (error) *error = "failed to add decoded take";
            return false;
        }
    }

    session = std::move(out);
    return true;
}

} // namespace

std::string JsonCodec::encodeFrame(const FacialFrame& frame) {
    return encodeFrameBody(frame);
}

bool JsonCodec::decodeFrame(const std::string& json, FacialFrame& frame, std::string* error) {
    JsonValue root;
    Parser parser(json);
    if (!parser.parse(root, error)) return false;
    return decodeFrameValue(root, frame, error);
}

std::string JsonCodec::encodeSequence(const FacialSequence& sequence) {
    std::ostringstream out;
    out << "{\"type\":\"BDFRSequence\",\"version\":1,\"frames\":[";
    bool first = true;
    for (const auto& frame : sequence.frames()) {
        if (!first) out << ',';
        first = false;
        out << encodeFrameBody(frame);
    }
    out << "]}";
    return out.str();
}

bool JsonCodec::decodeSequence(const std::string& json, FacialSequence& sequence, std::string* error) {
    JsonValue root;
    Parser parser(json);
    if (!parser.parse(root, error)) return false;
    const JsonValue* frames = member(root, "frames");
    if (!frames || frames->type != JsonValue::Type::Array) {
        if (error) *error = "sequence frames array missing";
        return false;
    }

    FacialSequence out;
    for (const auto& value : frames->array) {
        FacialFrame frame;
        if (!decodeFrameValue(value, frame, error) || !out.addFrame(std::move(frame))) {
            if (error && error->empty()) *error = "invalid sequence frame";
            return false;
        }
    }
    if (!out.validate(error)) return false;
    sequence = std::move(out);
    return true;
}

std::string JsonCodec::encodeSession(const Session& session) {
    std::ostringstream out;
    out << std::setprecision(12);
    out << "{\"type\":\"BDFRSession\",\"version\":1"
        << ",\"id\":\"" << escapeJson(session.id) << "\""
        << ",\"project\":\"" << escapeJson(session.project) << "\""
        << ",\"scene\":\"" << escapeJson(session.scene) << "\""
        << ",\"shot\":\"" << escapeJson(session.shot) << "\""
        << ",\"takes\":[";

    bool firstTake = true;
    for (const auto& take : session.takes) {
        if (!firstTake) out << ',';
        firstTake = false;
        out << "{\"id\":\"" << escapeJson(take.id) << "\""
            << ",\"name\":\"" << escapeJson(take.name) << "\""
            << ",\"actorId\":\"" << escapeJson(take.actorId) << "\""
            << ",\"source\":\"" << escapeJson(take.source) << "\""
            << ",\"duration\":" << take.durationSeconds
            << ",\"fpsNum\":" << take.frameRateNumerator
            << ",\"fpsDen\":" << take.frameRateDenominator
            << ",\"dirtyRanges\":[";

        bool firstRange = true;
        for (const auto& range : take.dirtyRanges) {
            if (!firstRange) out << ',';
            firstRange = false;
            out << "{\"start\":" << range.startSeconds << ",\"end\":" << range.endSeconds << "}";
        }
        out << "]}";
    }
    out << "]}";
    return out.str();
}

bool JsonCodec::decodeSession(const std::string& json, Session& session, std::string* error) {
    JsonValue root;
    Parser parser(json);
    if (!parser.parse(root, error)) return false;
    return decodeSessionValue(root, session, error);
}

std::string JsonCodec::encodeProject(const Project& project) {
    std::ostringstream out;
    out << "{\"type\":\"BDFRProject\",\"version\":" << project.schemaVersion
        << ",\"name\":\"" << escapeJson(project.name) << "\""
        << ",\"actors\":[";

    bool firstActor = true;
    for (const auto& actor : project.actors) {
        if (!firstActor) out << ',';
        firstActor = false;
        out << "{\"id\":\"" << escapeJson(actor.id)
            << "\",\"name\":\"" << escapeJson(actor.name)
            << "\",\"notes\":\"" << escapeJson(actor.notes) << "\"}";
    }

    out << "],\"sessions\":[";
    bool firstSession = true;
    for (const auto& session : project.sessions) {
        if (!firstSession) out << ',';
        firstSession = false;
        out << encodeSession(session);
    }
    out << "]}";
    return out.str();
}

bool JsonCodec::decodeProject(const std::string& json, Project& project, std::string* error) {
    JsonValue root;
    Parser parser(json);
    if (!parser.parse(root, error)) return false;

    double version = 0.0;
    Project out;
    if (!numberMember(root, "version", version) || !stringMember(root, "name", out.name)) {
        if (error) *error = "project metadata missing";
        return false;
    }
    out.schemaVersion = static_cast<std::uint32_t>(version);

    const JsonValue* actors = member(root, "actors");
    const JsonValue* sessions = member(root, "sessions");
    if (!actors || actors->type != JsonValue::Type::Array ||
        !sessions || sessions->type != JsonValue::Type::Array) {
        if (error) *error = "project actors/sessions missing";
        return false;
    }

    for (const auto& av : actors->array) {
        ActorProfile actor;
        if (!stringMember(av, "id", actor.id) ||
            !stringMember(av, "name", actor.name) ||
            !stringMember(av, "notes", actor.notes) ||
            !out.addActor(std::move(actor))) {
            if (error) *error = "invalid project actor";
            return false;
        }
    }

    for (const auto& sv : sessions->array) {
        Session session;
        if (!decodeSessionValue(sv, session, error) || !out.addSession(std::move(session))) {
            if (error && error->empty()) *error = "invalid project session";
            return false;
        }
    }

    project = std::move(out);
    return true;
}

} // namespace bdfr
