import { openDB } from 'idb';

let name = "MechnicalMidiMusic";
let storeName = "Playlist";
let version: number|undefined = 1;

export class FileDB {
    private db: any;

	constructor() {
        this.db = undefined;
	}

    async init() {
    if(this.db != undefined) return;
    this.db = await openDB(name, version, {
        upgrade(db) {
            db.createObjectStore(storeName);
        },
        })
    }

    //Return a list of files
    async loadAll() {
        let files = await this.db.getAll(storeName);
        return files;
    }

    async add(file: File) {
        this.db.put(storeName, file, file.name);
    }

    async remove(file: File) {
        await this.db.delete(storeName, file.name);
    }

    async clear() {
        await this.db.clear(storeName)
    }
}