export class Node {
    private elem;
    private next: any;

    constructor(elem: any) {
        this.elem = elem;
        this.next = null;
    }
    public getElement(){
        return(this.elem);
    }

    public getNext(){
        return(this.next);
    }

    public setNext(next: Node){
        this.next = next;
    }
}

export class LinkedList {
    private head :any = null;
    private len = 0;

    * [Symbol.iterator]() { // define the default iterator for this class
        let node  = this.head; // get first node
        while (node) {  // while we have not reached the end of the list
            yield node.elem;  // ... yield the current node's data
            node = node.next; // and move to the next node
        }
    }
    
    forEach(cb: any) { // Takes a callback argument.
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

    public removeAt(pos : number) {
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


    public insert(elem:any, pos:number) {
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

// let t = new LinkedList();
// t.append('asd'); // Works fine
// t.append(1);
// t.append(0);
// console.log(t); // LinkedList
// let tt = t.removeAt(1);
// console.log(t, 'tt', tt); // LinkedList, 'tt', 1
// t.insert('asd', 2);
// let ttt = t.insert('a', 1)
// console.log(ttt); // true
// console.log(t); // LinkedList
// console.log(t.toString()); //asda0