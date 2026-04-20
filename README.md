Restaurant Management System

A C++ terminal-based restaurant management system with user and admin functionalities. This system handles menu management, order placement, order tracking, and token-based service management.

Features

👥 User Features

- **OTP-based Login**: Secure login with One-Time Password verification
- **Browse Menu**: View menu items organized by categories (Veg, Non-Veg, Desserts, Snacks, Drinks)
- **Shopping Cart**: Add/manage items in cart before checkout
- **Place Orders**: Place orders with automatic stock deduction
- **Order Tracking**: Real-time tracking of order status with token number and estimated ready time
- **Order History**: View past completed orders and reorder from history
- **Queue Management**: Intelligent queue tracking with estimated preparation time calculation

👨‍💼 Admin Features

- **Admin Panel**: Secure login (credentials: admin/password)
- **Order Management**: Accept, process, and track all orders
- **Status Updates**: Update order status through workflow (Pending → Accepted → Preparing → Ready → Completed)
- **Menu Management**: Add new items, delete items, view current menu
- **Inventory Control**: Automatic stock management and restocking on order cancellation
- **Order Cancellation**: Cancel orders and automatically restock items
- **View All Orders**: Monitor all orders in the system with details

📊 Order Status Workflow

- **PENDING**: Order placed, awaiting admin approval
- **ACCEPTED**: Order accepted with token number assigned
- **PREPARING**: Kitchen preparing the order
- **READY**: Order ready for pickup
- **COMPLETED**: Order completed and served
- **CANCELLED**: Order cancelled (items restocked)

Technical Details

Prerequisites

- C++ compiler (g++ recommended)
- Windows/Linux/Mac operating system
- Terminal/Command prompt access

### Project Structure

```
├── main.cpp           # Main application source code
├── menu.txt          # Menu data storage
├── orders.txt        # Orders data storage
├── orderid.txt       # Next order ID tracker
├── token.txt         # Next token number tracker
├── README.md         # This file
└── restraurantmanagement.exe (compiled binary)
```

### Data Storage Format

**menu.txt**: `id|name|category|price|quantity|prepTime`

- Example: `1|Veg Thali|Veg|90.0|10|15`

**orders.txt**: `orderId|userName|mobile|status|tokenNumber|placedAt|totalAmount|estimatedReadyAt|items`

- Example: `1|John|9876543210|1|101|1713628800|180.0|1713629700|1:2,2:1`

## Build & Run

**Step 1: Verify Requirements**
Check that g++ is installed (see `requirements.txt`):

```bash
g++ --version
```

**Step 2: Compile**

```bash
g++ -fdiagnostics-color=always -g main.cpp -o restraurantmanagement.exe
```

Or use VS Code: Press `Ctrl+Shift+B`

**Step 3: Execute**

```bash
./restraurantmanagement.exe
```

On Windows: `restraurantmanagement.exe`

## Usage Guide

### Main Menu

```
=== Smart Restaurant Console ===
1. User
2. Admin
3. Exit
```

### User Flow

1. **Login**
   - Enter name and mobile number
   - System generates OTP (shown in debug for testing)
   - Enter OTP to verify (valid for 120 seconds)

2. **Place Order**
   - View menu by category
   - Select items and quantities
   - Add items to cart
   - View cart total
   - Place order and receive Order ID & Token Number
   - Track order in real-time

3. **Track Order**
   - Enter Order ID to check current status
   - View token number and estimated ready time
   - Refresh to get updates

4. **Order History**
   - View all completed orders
   - Reorder from previous purchases

### Admin Flow

1. **Login**
   - Admin ID: `admin`
   - Password: `password`

2. **View Orders**
   - See all orders with details
   - Select order to manage
   - Update status based on workflow
   - Assign token numbers

3. **Add Menu Item**
   - Enter item name, category, price
   - Set quantity and prep time
   - Item gets auto-assigned ID

4. **Delete Menu Item**
   - Enter item ID to remove
   - Item removed from menu

5. **View Menu**
   - See all menu items
   - Check availability and quantities

## Default Menu Items

The system comes with a pre-configured menu:

| ID  | Item            | Category | Price  | Prep Time |
| --- | --------------- | -------- | ------ | --------- |
| 1   | Veg Thali       | Veg      | Rs.90  | 15 min    |
| 2   | Paneer Masala   | Veg      | Rs.120 | 20 min    |
| 3   | Chicken Biryani | Non-Veg  | Rs.150 | 25 min    |
| 4   | Mutton Curry    | Non-Veg  | Rs.200 | 30 min    |
| 5   | Gulab Jamun     | Desserts | Rs.40  | 5 min     |
| 6   | Ice Cream       | Desserts | Rs.35  | 2 min     |
| 7   | Masala Dosa     | Snacks   | Rs.50  | 12 min    |
| 8   | Samosa          | Snacks   | Rs.20  | 8 min     |
| 9   | Tea             | Drinks   | Rs.10  | 3 min     |
| 10  | Cold Drink      | Drinks   | Rs.40  | 1 min     |

## Key Algorithms

### Estimated Ready Time Calculation

```
Base Prep Time = Maximum prep time among items in order
Queue Delay = Number of active orders ahead × 2 minutes
Total Estimate = Current Time + (Base Prep + Queue Delay) × 60 seconds
```

### Stock Management

- Stock is deducted when order is placed
- Stock is restored if order is cancelled
- Items marked as "Not Available" when qty = 0

## File Persistence

All data is automatically saved to files:

- Orders are saved after each order placement/status update
- Menu changes are saved immediately
- Order ID and token counters are incremented and saved

## Security Notes

- OTP expires after 120 seconds
- Admin credentials are hardcoded (for demo purposes)
- Mobile number and name used for order association
- Debug OTP display (for testing - can be removed in production)

## Possible Enhancements

- Database integration instead of text files
- User registration and password management
- Email/SMS notifications
- Payment gateway integration
- Advanced reporting and analytics
- Multi-location support
- Customer ratings and reviews

## Troubleshooting

**"Invalid input" message when entering numbers**

- Ensure you enter valid numeric values
- Avoid special characters

**Orders not saving**

- Check file permissions in the directory
- Ensure disk space is available

**OTP not working**

- OTP is case-sensitive
- Check debug message for correct OTP
- Verify OTP within 120 seconds

## License

This is a demonstration project for learning purposes.

## Author

Restaurant Management System - C++ Project
