# notepad.fy — JSON-backed notepad using Funcy stdlib I/O & JSON.

class Notepad {
    func &Notepad(path="notes.json", log_path="notes.log") {
        &path = path;
        &log_path = log_path;
        &notes = [];      # list of { id, text, done, tags }
        &next_id = 1;
        &load();
    }

    func &log(msg) {
        timestamp = "<log> ";
        appendFile(&log_path, timestamp + msg + "\n");
    }

    func &load() {
        raw = readFile(&path);
        if raw == "" {
            &notes = [];
            &next_id = 1;
            return;
        }
        data = raw.toJson();              # expect a dict
        if "notes" in data { &notes = data["notes"]; } else { &notes = []; }
        if "next_id" in data { &next_id = data["next_id"]; }
        if &next_id < 1 {
            # recompute if missing/corrupt
            max_id = 0;
            for n in &notes { if n["id"] > max_id { max_id = n["id"]; } }
            &next_id = max_id + 1;
        }
    }

    func &save() {
        doc = { "next_id": &next_id, "notes": &notes };
        writeFile(&path, str(doc));       # stringify dict -> JSON
        &log("saved " + str(length(&notes)) + " notes");
    }

    func &add(text, tags=[]) {
        note = { "id": &next_id, "text": text, "done": false, "tags": tags };
        &notes.append(note);
        &next_id += 1;
        &save();
        return note["id"];
    }

    func &toggle(id) {
        for i in range(length(&notes)) {
            if &notes[i]["id"] == id {
                &notes[i]["done"] = not &notes[i]["done"];
                &save();
                return;
            }
        }
        throw "No note with id " + str(id);
    }

    func &remove(id) {
        out = [];
        removed = false;
        for n in &notes {
            if n["id"] == id { removed = true; } else { out.append(n); }
        }
        if not removed { throw "No note with id " + str(id); }
        &notes = out;
        &save();
    }

    func &list(filter="all") {
        if filter == "all" { return &notes; }
        if filter == "open" {
            out = [];
            for n in &notes { if not n["done"] { out.append(n); } }
            return out;
        }
        if filter == "done" {
            out = [];
            for n in &notes { if n["done"] { out.append(n); } }
            return out;
        }
        if filter.startsWith("tag:") {
            want = filter.split(":")[1];
            out = [];
            for n in &notes { if want in n["tags"] { out.append(n); } }
            return out;
        }
        return &notes;
    }

    func &prettyPrint(notes) {
        for n in notes {
            status = "";
            if n["done"] {
                status = "[x]";
            } else {
                status = "[ ]";
            }
            tag_s = "";
            if length(n["tags"]) == 0 {
                tag_s = "(no tags)";
            } else {
                tag_s = "#" + ", #".join(n["tags"]);
            }
            print(n["id"], status, "-", n["text"], "-", tag_s);
        }
    }
}

# --- Demo ---
np = Notepad(path="notes.json");

print("Add a few notes…");
id1 = np.add("Finish Funcy demo", ["work", "funcy"]);
id2 = np.add("Buy milk", ["home"]);
id3 = np.add("Read a short story", ["leisure"]);

print("\nAll notes:");
np.prettyPrint(np.list("all"));

print("\nToggle second note, list open:");
np.toggle(id2);
np.prettyPrint(np.list("open"));

print("\nFilter by tag 'funcy':");
np.prettyPrint(np.list("tag:funcy"));

print("\nRemove note #", id3, "and show all:");
np.remove(id3);
np.prettyPrint(np.list("all"));

print("\nRaw JSON on disk:");
print(readFile("notes.json"));
