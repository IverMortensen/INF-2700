-- write settings and queries here and run
-- sqlite3 inf2700_orders.sqlite3 < assignment1_queries.sql
-- to perform the queries
-- for example:

.mode column
.header on


---- Task 2 -----

SELECT customerName, contactLastName, contactFirstName
FROM Customers;

SELECT *
FROM Orders
WHERE shippedDate IS NULL;

SELECT C.customerName AS Customer, SUM(OD.quantityOrdered) AS Total
FROM Orders O, Customers C, OrderDetails OD
WHERE O.customerNumber = C.customerNumber
AND O.orderNumber = OD.orderNumber
GROUP BY O.customerNumber
ORDER BY Total DESC;

SELECT P.productName, T.totalQuantityOrdered
FROM Products P NATURAL JOIN
(SELECT productCode, SUM(quantityOrdered) AS totalQuantityOrdered
FROM OrderDetails GROUP BY productCode) AS T
WHERE T.totalQuantityOrdered >= 1000;

SELECT DISTINCT productName, productVendor
FROM   Products
LIMIT  6;


---- Task 3 ----

 -- 1 --
SELECT customerName, country
FROM Customers
WHERE country LIKE '%Norway%';

 -- 2 --
SELECT productName, productScale
FROM Products
WHERE productLine LIKE '%Classic Cars%';

 -- 3 --
SELECT O.orderNumber, O.requiredDate, OD.quantityOrdered, P.productName, P.quantityInStock
FROM Orders O, OrderDetails OD, Products P
WHERE O.status LIKE '%In Process%'
AND OD.orderNumber == O.orderNumber
AND P.productCode == OD.productCode;

 -- 4 --
SELECT 
    C.customerName,
    C.creditLimit,
    total_price,
    total_payments, 
	(total_price - total_payments) AS difference
FROM 
    Customers C
	
JOIN (
    SELECT 
        O.customerNumber, 
        SUM(OD.priceEach * OD.quantityOrdered) AS total_price
    FROM 
        OrderDetails OD
    JOIN 
        Orders O ON O.orderNumber = OD.orderNumber
    GROUP BY 
        O.customerNumber
) TotalPrices ON C.customerNumber = TotalPrices.customerNumber

JOIN (
    SELECT 
        P.customerNumber, 
        SUM(P.amount) AS total_payments
    FROM 
        Payments P
    GROUP BY 
        P.customerNumber
) TotalPayments ON C.customerNumber = TotalPayments.customerNumber

WHERE (total_price - total_payments) > creditLimit;

-- 5 --
-- Select customer names from customer numbers
SELECT C.customerName
FROM Customers C
WHERE C.customerNumber IN
	(
	-- Select all customer numbers that have ordered the same product codes
	SELECT O.customerNumber
	FROM OrderDetails OD
	JOIN Orders O ON O.orderNumber = OD.orderNumber
	WHERE OD.productCode IN
		(
		-- Select all product codes orderd by 219
		SELECT OD.productCode
		FROM OrderDetails OD
		JOIN Orders O ON O.orderNumber = OD.orderNumber
		WHERE O.customerNumber = 219
		)
	)
ORDER BY C.customerName;

