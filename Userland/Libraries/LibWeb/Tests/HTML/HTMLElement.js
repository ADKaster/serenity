loadPage("file:///res/html/misc/welcome.html");

afterInitialPageLoad(() => {
    test("contentEditable attribute", () => {
        expect(document.body.contentEditable).toBe("inherit");
        expect(document.firstChild.nextSibling.nodeName).toBe("HTML");
        expect(document.firstChild.nextSibling.contentEditable).toBe("true");
    });
});
