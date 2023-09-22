//Playlist is a Linked List with modified iterator.

export class Node {
	private elem: Song;
	private next: Node;
	private prev: Node;

	constructor(elem: any) {
		this.elem = elem;
		this.next = this;
		this.prev = this;
	}
	public getElement() {
		return this.elem;
	}

	public getNext() {
		return this.next;
	}
	public getPrev() {
		return this.prev;
	}

	public setNext(next: Node) {
		this.next = next;
	}

	public setPrev(prev: Node) {
		this.prev = prev;
	}
}

export class Playlist {
	private head: Node|null = null;
	private len = 0;

	*[Symbol.iterator]() {
		// define the default iterator for this class
		let node = this.head; // get first node
		if (node == null) return;
		let i = 0;
		while (i < this.len) {
			// while we have not reached the end of the list
			yield node; // ... yield the current node's data
			node = node.getNext(); // and move to the next node
			i++;
		}
	}

	forEach(cb: any) {
		// Takes a callback argument.
		// As this object has a default iterator (see above) we
		// can use the following syntax to consume that iterator.
		// Call the callback for each yielded value.
		for (let data of this) cb(data);
	}

	public append(elem: Song) {
		let node = new Node(elem);
		if (this.head == null) {
			this.head = node;
		} else { // Insert node before head
			let last = this.head.getPrev();
			this.head.setPrev(node);
			last.setNext(node);
			node.setPrev(last);
			node.setNext(this.head)
		}
		this.len++;
	}

	//Returns the Next song.
	public remove(song:Song) {

		if(this.head == null) return null;
		let current = this.head;
		let previous = this.head.getPrev();

		for (let i = 0; i < this.len; i++){
			
			if (song == current.getElement()){
				this.len--;
				if(current == this.head){
					this.head = current.getNext();
					if(this.len == 0){
						this.head = null;
						return null;
					}
				}
				let next = current.getNext();
				previous.setNext(next);
				next.setPrev(previous);
				return next;
			}
			previous = current;
			current = current.getNext();
		}
		return null;
	}

	public toString() {
		let str = '';
		this.forEach((node: Node) => str += (node.getElement().title) + ", ");
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
	public file: File | null;
	public title: string;
	public time: number;
	//--------- TODO ---------
	constructor(file: File | null) {
		this.file = file;
		this.title = (file === null) ? '' : file.name.split(".")[0];
		this.time = 0;
	}

	public getData(){
		return this.file?.arrayBuffer();
	}
}