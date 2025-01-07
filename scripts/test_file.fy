/* Library Management System */

# Define a Book class
class Book {
    func &Book(title, author, year) {
        &title = title;
        &author = author;
        &year = year;
        &is_checked_out = false;
    }

    func &getInfo() {
        return "Title: " + &title + ", Author: " + &author + ", Year: " + str(&year);
    }

    func &checkout() {
        if &is_checked_out {
            print(&title + " is already checked out.");
        } else {
            &is_checked_out = true;
            print("You have checked out " + &title);
        }
    }

    func &returnBook() {
        if &is_checked_out {
            &is_checked_out = false;
            print("You have returned " + &title);
        } else {
            print(&title + " was not checked out.");
        }
    }
}

# Define a Library class
class Library {
    func &Library() {
        &books = list();
    }

    func &addBook(book) {
        &books.append(book);
        print("Added: " + book.getInfo());
    }

    func &listBooks() {
        if &books.size() == 0 {
            print("The library has no books.");
        } else {
            print("Books in the library:");
            for book in &books {
                print("- " + book.getInfo());
            }
        }
    }

    func &checkoutBook(title) {
        for book in &books {
            if book.title == title {
                book.checkout();
                return;
            }
        }
        print("Book not found: " + title);
    }

    func &returnBook(title) {
        for book in &books {
            if book.title == title {
                book.returnBook();
                return;
            }
        }
        print("Book not found: " + title);
    }
}

# Main program
func main() {
    library = Library();

    # Add books to the library
    book1 = Book("The Great Gatsby", "F. Scott Fitzgerald", 1925);
    book2 = Book("1984", "George Orwell", 1949);
    book3 = Book("To Kill a Mockingbird", "Harper Lee", 1960);

    library.addBook(book1);
    library.addBook(book2);
    library.addBook(book3);

    # List all books
    library.listBooks();

    # Checkout and return books
    library.checkoutBook("1984");
    library.checkoutBook("The Great Gatsby");
    library.returnBook("1984");

    # Try to checkout an already checked-out book
    library.checkoutBook("The Great Gatsby");

    # Return a book that wasn't checked out
    library.returnBook("To Kill a Mockingbird");

    # List books again to see the status
    library.listBooks();
}

# Run the program
main();
