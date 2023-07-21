//Playlist is a Linked List with modified iterator.

export class Node {
	private elem;
	private next: any;

	constructor(elem: any) {
		this.elem = elem;
		this.next = null;
	}
	public getElement() {
		return this.elem;
	}

	public getNext() {
		return this.next;
	}

	public setNext(next: Node) {
		this.next = next;
	}
}

export class Playlist {
	private head: any = null;
	private len = 0;

	*[Symbol.iterator]() {
		// define the default iterator for this class
		let node = this.head; // get first node
		while (node) {
			// while we have not reached the end of the list
			yield node; // ... yield the current node's data
			node = node.next; // and move to the next node
		}
	}

	forEach(cb: any) {
		// Takes a callback argument.
		// As this object has a default iterator (see above) we
		//    can use the following syntax to consume that iterator.
		// Call the callback for each yielded value.
		for (let data of this) cb(data);
	}

	public append(elem: any) {
		let node = new Node(elem);
		let current;

		if (this.head === null) {
			this.head = node;
		} else {
			current = this.head;
			while (current.next) {
				current = current.next;
			}
			current.next = node;
		}
		this.len++;
	}

	public removeAt(pos: number) {
		if (pos > -1 && pos < this.len) {
			let current = this.head;
			let previous;
			let index = 0;

			if (pos === 0) {
				this.head = current.next;
			} else {
				while (index++ < pos) {
					previous = current;
					current = current.next;
				}
				previous.next = current.next;
			}
			this.len--;
			return current.elem;
		} else {
			return null;
		}
	}

	public insert(elem: any, pos: number) {
		if (pos > -1 && pos < this.len) {
			let current = this.head;
			let index = 0;
			let previous;
			let node = new Node(elem);

			if (pos === 0) {
				node.setNext(current);
				this.head = node;
			} else {
				while (index++ < pos) {
					previous = current;
					current = current.next;
				}
				node.setNext(current);
				previous.next = node;
			}
			this.len++;
			return true;
		} else {
			return false;
		}
	}

	public toString() {
		var current = this.head;
		var str = '';
		while (current) {
			str += current.elem; //output is undefinedundefinedundefined
			// str += JSON.stringify(current);
			// prints out {"next":{"next":{}}}{"next":{}}{}
			current = current.next;
		}
		return str;
	}

	public getHead() {
		return this.head;
	}

	public getLen() {
		return this.len;
	}
}

export class Song {
	public path: string;
	public title: string;
	public time: number;
	//--------- TODO ---------
	constructor(path: string | null) {
		if (path === null) path = '';
		this.path = path;
		this.title = path;
		this.time = 0;
	}
}

// export let playlist = new LinkedList();
// export let curSong = new Song(null);

// let curNode: Node | null;

// export function setNode(node: Node){
//     curNode = node;
//     curSong = node.getElement();
//     console.log(curSong);
// }

// export function setSong(song: Song){
//     if(curSong !== song){
//         //curSec = 0;
//     }
//     curSong = song;
// }

// // Add Song
// export function addSong(filePath: string){
//     let song = new Song(filePath);
//     if(curSong===null){
//         curSong=song
//         curNode = playlist.getHead();
//     };
//     playlist.append(song);
// }
// // Remove Song
// export function removeSong(pos: number){
//     let oldNode = playlist.removeAt(pos);
//     if (curNode === oldNode){
//         if(oldNode.getNext() !== null){
//             curNode = oldNode.getNext();
//         }
//         else{
//             curNode = playlist.getHead();
//         }
//         if(curNode !== null) setSong(curNode.getElement());
//     }
// }
// export function addPlayList(filePath: string){
//     //For song
//     addSong(filePath);
// }
// export function clearPlayList(){
//     // JS will free out of scope memory
//     playlist = new LinkedList();
//     curSong = new Song(null);
//     curNode = null;
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////
// //-------------------------------------------- DEV --------------------------------------------//
// /////////////////////////////////////////////////////////////////////////////////////////////////

// let song1 = new Song('C:Desktop\songs\Montero (call me by your name) by Lil Nas X');
// song1.title = 'Montero (call me by your name) by Lil Nas X';
// song1.time = 299;
// let song2 = new Song('C:Desktop\songs\Head & Heart by Joel Corry ft Mnek');
// song2.title = 'Head & Heart by Joel Corry ft Mnek';
// song2.time = 158;
// let song3 = new Song('C:Desktop\songs\You broke me first by Tate Mcrae');
// song3.title = 'You broke me first by Tate Mcrae';
// song3.time = 200;
// let song4 = new Song('C:Desktop\songs\Lean On by Major Lazer & Dj shake ft. Mo');
// song4.title = 'Lean On by Major Lazer & Dj shake ft. Mo';
// song4.time = 174;
// playlist.append(song1);
// playlist.append(song2);
// playlist.append(song3);
// playlist.append(song4);
