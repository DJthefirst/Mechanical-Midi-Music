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
    if (this.db != undefined) return;
    try {
        this.db = await openDB(name, version, {
            upgrade(db) {
                if (!db.objectStoreNames.contains(storeName)) {
                    db.createObjectStore(storeName);
                }
            },
        });
        console.log('[FileDB] IndexedDB initialized:', name, 'store:', storeName);
    } catch (err) {
        console.error('[FileDB] Failed to open IndexedDB:', err);
        this.db = undefined;
        throw err;
    }
    }

    //Return a list of files
    async loadAll() {
        if (!this.db) {
            console.warn('[FileDB] loadAll called but DB is not initialized');
            return [];
        }
        try {
            const files = await this.db.getAll(storeName);
            console.log('[FileDB] loadAll returned', files?.length ?? 0, 'items');
            return files;
        } catch (err) {
            console.error('[FileDB] loadAll error:', err);
            return [];
        }
    }

    async add(file: File) {
        if (!this.db) {
            console.warn('[FileDB] add called but DB is not initialized');
            return;
        }
        try {
            await this.db.put(storeName, file, file.name);
            console.log('[FileDB] Added file to DB:', file.name);
        } catch (err) {
            console.error('[FileDB] add error:', err);
        }
    }

    async remove(file: File) {
        if (!this.db) return;
        try {
            await this.db.delete(storeName, file.name);
            console.log('[FileDB] Removed file from DB:', file.name);
        } catch (err) {
            console.error('[FileDB] remove error:', err);
        }
    }

    async clear() {
        if (!this.db) return;
        try {
            await this.db.clear(storeName);
            console.log('[FileDB] Cleared store:', storeName);
        } catch (err) {
            console.error('[FileDB] clear error:', err);
        }
    }
}