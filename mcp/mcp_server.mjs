/**
 * MCP Server for GraphQL Bookstore API
 * Allows AI/LLM models to interact with the Bookstore GraphQL API via MCP protocol.
 * 
 * Usage:
 *   npm install
 *   npm start
 * 
 * Or with custom API URL:
 *   API_URL=http://localhost:4000/graphql npm start
 */

import { fileURLToPath } from 'url';
import { dirname, join } from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const { Server } = await import(join(__dirname, 'node_modules/@modelcontextprotocol/sdk/dist/server/index.js'));
const { StdioServerTransport } = await import(join(__dirname, 'node_modules/@modelcontextprotocol/sdk/dist/server/stdio.js'));
const { CallToolRequestSchema } = await import(join(__dirname, 'node_modules/@modelcontextprotocol/sdk/dist/types.js'));
const { ListResourcesRequestSchema } = await import(join(__dirname, 'node_modules/@modelcontextprotocol/sdk/dist/types.js'));
const { ListToolsRequestSchema } = await import(join(__dirname, 'node_modules/@modelcontextprotocol/sdk/dist/types.js'));
const { ReadResourceRequestSchema } = await import(join(__dirname, 'node_modules/@modelcontextprotocol/sdk/dist/types.js'));

const API_URL = process.env.API_URL || 'http://127.0.0.1:4000/graphql';
const DEFAULT_USERNAME = 'admin';
const DEFAULT_PASSWORD = 'password123';

class BookstoreMCPClient {
  constructor() {
    this.token = null;
  }

  async _execute(query, variables = {}) {
    const headers = { 'Content-Type': 'application/json' };
    if (this.token) {
      headers['Authorization'] = `Bearer ${this.token}`;
    }

    const response = await fetch(API_URL, {
      method: 'POST',
      headers,
      body: JSON.stringify({ query, variables }),
    });

    if (!response.ok) {
      throw new Error(`HTTP error: ${response.status}`);
    }

    const result = await response.json();
    if (result.errors) {
      throw new Error(JSON.stringify(result.errors));
    }
    return result.data;
  }

  async login(username = DEFAULT_USERNAME, password = DEFAULT_PASSWORD) {
    const query = `
      mutation {
        login(username: "${username}", password: "${password}") {
          success
          token
          message
        }
      }
    `;
    const data = await this._execute(query);
    if (data.login.success) {
      this.token = data.login.token;
    }
    return data.login;
  }

  async register(username, firstName, lastName, password) {
    const query = `
      mutation {
        register(username: "${username}", firstName: "${firstName}", lastName: "${lastName}", password: "${password}") {
          success
          message
          userId
        }
      }
    `;
    return this._execute(query);
  }

  async me() {
    const query = `
      query Me {
        me {
          id
          username
          firstName
          lastName
          role
        }
      }
    `;
    return this._execute(query);
  }

  async books(search = '', categoryId = 0) {
    const searchStr = search ? `"${search}"` : '""';
    const query = `
      query Books {
        books(search: ${searchStr}, categoryId: ${categoryId}) {
          id
          title
          author
          price
          category
          description
        }
      }
    `;
    return this._execute(query);
  }

  async book(id) {
    const query = `
      query Book {
        book(id: ${id}) {
          id
          title
          author
          price
          category
          description
          stock
        }
      }
    `;
    return this._execute(query, { id });
  }

  async cart() {
    const query = `
      query Cart {
        cart {
          id
          items {
            bookId
            title
            quantity
            price
          }
          subtotal
          tax
          total
        }
      }
    `;
    return this._execute(query);
  }

  async addToCart(bookId, quantity = 1) {
    const query = `
      mutation {
        addToCart(bookId: ${bookId}, quantity: ${quantity}) {
          success
          message
        }
      }
    `;
    return this._execute(query);
  }

  async removeFromCart(bookId) {
    const query = `
      mutation {
        removeFromCart(bookId: ${bookId}) {
          success
          message
        }
      }
    `;
    return this._execute(query);
  }

  async orders() {
    const query = `
      query Orders {
        orders {
          id
          status
          totalAmount
          createdAt
          items {
            bookId
            title
            quantity
            price
          }
        }
      }
    `;
    return this._execute(query);
  }

  async createOrder() {
    const query = `
      mutation {
        createOrder {
          success
          orderId
          totalAmount
          message
        }
      }
    `;
    return this._execute(query);
  }

  async checkout(cardNumber, expiry, cvv) {
    const query = `
      mutation {
        checkout(cardNumber: "${cardNumber}", expiry: "${expiry}", cvv: "${cvv}") {
          success
          orderId
          message
        }
      }
    `;
    return this._execute(query);
  }

  async cancelOrder(orderId) {
    const query = `
      mutation {
        cancelOrder(orderId: "${orderId}") {
          success
          message
        }
      }
    `;
    return this._execute(query);
  }

  async createReview(bookId, rating, comment) {
    const query = `
      mutation {
        createReview(bookId: ${bookId}, rating: ${rating}, comment: "${comment}") {
          success
          reviewId
          message
        }
      }
    `;
    return this._execute(query);
  }

  async deleteReview(reviewId) {
    const query = `
      mutation {
        deleteReview(reviewId: ${reviewId}) {
          success
          message
        }
      }
    `;
    return this._execute(query);
  }

  async bookReviews(bookId) {
    const query = `
      query BookReviews {
        bookReviews(bookId: ${bookId}) {
          id
          userId
          rating
          comment
          createdAt
        }
      }
    `;
    return this._execute(query);
  }

  async myReviews() {
    const query = `
      query MyReviews {
        myReviews {
          id
          bookId
          rating
          comment
          createdAt
        }
      }
    `;
    return this._execute(query);
  }

  async updateProfile(firstName, lastName) {
    const query = `
      mutation {
        updateProfile(firstName: "${firstName}", lastName: "${lastName}") {
          success
          message
        }
      }
    `;
    return this._execute(query);
  }

  async searchBooks(query) {
    return this.books(query);
  }

  async applyCoupon(code) {
    const query = `
      mutation {
        applyCoupon(code: "${code}") {
          success
          message
          discount
        }
      }
    `;
    return this._execute(query);
  }

  async registerWebhook(url, events, secret = '') {
    const eventsStr = '["' + events.join('","') + '"]';
    const query = `
      mutation {
        registerWebhook(url: "${url}", events: ${eventsStr}, secret: "${secret}") {
          success
          webhookId
          message
        }
      }
    `;
    return this._execute(query);
  }

  async webhooks() {
    const query = `
      query Webhooks {
        webhooks {
          id
          url
          events
          createdAt
        }
      }
    `;
    return this._execute(query);
  }

  async testWebhook(webhookId) {
    const query = `
      mutation {
        testWebhook(webhookId: ${webhookId}) {
          success
          message
        }
      }
    `;
    return this._execute(query);
  }

  async logout() {
    const query = `
      mutation {
        logout {
          success
          message
        }
      }
    `;
    const data = await this._execute(query);
    this.token = null;
    return data.logout;
  }

  async adminStats() {
    const query = `
      query AdminStats {
        _adminStats {
          userCount
          bookCount
          totalRevenue
        }
      }
    `;
    return this._execute(query);
  }

  async adminOrders() {
    const query = `
      query AdminOrders {
        _adminAllOrders {
          id
          userId
          status
          totalAmount
          createdAt
        }
      }
    `;
    return this._execute(query);
  }

  async adminPayments() {
    const query = `
      query AdminPayments {
        _adminAllPayments {
          id
          orderId
          amount
          status
          createdAt
        }
      }
    `;
    return this._execute(query);
  }

  async proInventory() {
    const query = `
      query ProInventory {
        _proInventory {
          id
          title
          author
          price
          difficulty
          hint
        }
      }
    `;
    return this._execute(query);
  }
}

const client = new BookstoreMCPClient();

const tools = [
  {
    name: 'bookstore_login',
    description: 'Login to the Bookstore API with username and password',
    inputSchema: {
      type: 'object',
      properties: {
        username: { type: 'string', description: 'Username (default: admin)' },
        password: { type: 'string', description: 'Password (default: password123)' },
      },
    },
  },
  {
    name: 'bookstore_register',
    description: 'Register a new user account',
    inputSchema: {
      type: 'object',
      properties: {
        username: { type: 'string', description: 'Desired username' },
        firstName: { type: 'string', description: 'User first name' },
        lastName: { type: 'string', description: 'User last name' },
        password: { type: 'string', description: 'Desired password' },
      },
      required: ['username', 'firstName', 'lastName', 'password'],
    },
  },
  {
    name: 'bookstore_me',
    description: 'Get current authenticated user profile',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'bookstore_books',
    description: 'List all books with optional search and category filter',
    inputSchema: {
      type: 'object',
      properties: {
        search: { type: 'string', description: 'Search query for book title or author' },
        categoryId: { type: 'number', description: 'Filter by category ID (0 for all)' },
      },
    },
  },
  {
    name: 'bookstore_book',
    description: 'Get a specific book by ID',
    inputSchema: {
      type: 'object',
      properties: {
        id: { type: 'number', description: 'Book ID' },
      },
      required: ['id'],
    },
  },
  {
    name: 'bookstore_cart',
    description: 'Get current user shopping cart',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'bookstore_add_to_cart',
    description: 'Add a book to shopping cart',
    inputSchema: {
      type: 'object',
      properties: {
        bookId: { type: 'number', description: 'Book ID to add' },
        quantity: { type: 'number', description: 'Quantity (default: 1)' },
      },
      required: ['bookId'],
    },
  },
  {
    name: 'bookstore_remove_from_cart',
    description: 'Remove a book from shopping cart',
    inputSchema: {
      type: 'object',
      properties: {
        bookId: { type: 'number', description: 'Book ID to remove' },
      },
      required: ['bookId'],
    },
  },
  {
    name: 'bookstore_orders',
    description: 'Get current user order history',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'bookstore_create_order',
    description: 'Create an order from current cart',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'bookstore_checkout',
    description: 'Process checkout with payment',
    inputSchema: {
      type: 'object',
      properties: {
        cardNumber: { type: 'string', description: 'Credit card number (default: test card)' },
        expiry: { type: 'string', description: 'Expiry date (default: 12/25)' },
        cvv: { type: 'string', description: 'CVV (default: 123)' },
      },
    },
  },
  {
    name: 'bookstore_cancel_order',
    description: 'Cancel an order',
    inputSchema: {
      type: 'object',
      properties: {
        orderId: { type: 'string', description: 'Order ID to cancel' },
      },
      required: ['orderId'],
    },
  },
  {
    name: 'bookstore_create_review',
    description: 'Create a book review',
    inputSchema: {
      type: 'object',
      properties: {
        bookId: { type: 'number', description: 'Book ID to review' },
        rating: { type: 'number', description: 'Rating (1-5)' },
        comment: { type: 'string', description: 'Review comment' },
      },
      required: ['bookId', 'rating', 'comment'],
    },
  },
  {
    name: 'bookstore_delete_review',
    description: 'Delete a review',
    inputSchema: {
      type: 'object',
      properties: {
        reviewId: { type: 'number', description: 'Review ID to delete' },
      },
      required: ['reviewId'],
    },
  },
  {
    name: 'bookstore_book_reviews',
    description: 'Get reviews for a specific book',
    inputSchema: {
      type: 'object',
      properties: {
        bookId: { type: 'number', description: 'Book ID' },
      },
      required: ['bookId'],
    },
  },
  {
    name: 'bookstore_my_reviews',
    description: 'Get current user reviews',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'bookstore_update_profile',
    description: 'Update user profile',
    inputSchema: {
      type: 'object',
      properties: {
        firstName: { type: 'string', description: 'New first name' },
        lastName: { type: 'string', description: 'New last name' },
      },
    },
  },
  {
    name: 'bookstore_search',
    description: 'Search books by title or author',
    inputSchema: {
      type: 'object',
      properties: {
        query: { type: 'string', description: 'Search query' },
      },
      required: ['query'],
    },
  },
  {
    name: 'bookstore_apply_coupon',
    description: 'Apply coupon code to cart',
    inputSchema: {
      type: 'object',
      properties: {
        code: { type: 'string', description: 'Coupon code' },
      },
      required: ['code'],
    },
  },
  {
    name: 'bookstore_register_webhook',
    description: 'Register a webhook URL for events',
    inputSchema: {
      type: 'object',
      properties: {
        url: { type: 'string', description: 'Webhook URL' },
        events: { type: 'array', items: { type: 'string' }, description: 'Events to subscribe' },
        secret: { type: 'string', description: 'Webhook secret' },
      },
      required: ['url', 'events'],
    },
  },
  {
    name: 'bookstore_webhooks',
    description: 'List user webhooks',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'bookstore_test_webhook',
    description: 'Test a webhook',
    inputSchema: {
      type: 'object',
      properties: {
        webhookId: { type: 'number', description: 'Webhook ID to test' },
      },
      required: ['webhookId'],
    },
  },
  {
    name: 'bookstore_logout',
    description: 'Logout and clear token',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'bookstore_admin_stats',
    description: 'Get admin statistics',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'bookstore_admin_orders',
    description: 'Get all orders (admin)',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'bookstore_admin_payments',
    description: 'Get all payments (admin)',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'bookstore_pro_inventory',
    description: 'Get pro/hidden inventory (expert-level books)',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
];

async function handleToolCall(toolName, args) {
  try {
    switch (toolName) {
      case 'bookstore_login':
        return await client.login(args.username, args.password);
      case 'bookstore_register':
        return await client.register(args.username, args.firstName, args.lastName, args.password);
      case 'bookstore_me':
        return await client.me();
      case 'bookstore_books':
        return await client.books(args.search || '', args.categoryId || 0);
      case 'bookstore_book':
        return await client.book(args.id);
      case 'bookstore_cart':
        return await client.cart();
      case 'bookstore_add_to_cart':
        return await client.addToCart(args.bookId, args.quantity || 1);
      case 'bookstore_remove_from_cart':
        return await client.removeFromCart(args.bookId);
      case 'bookstore_orders':
        return await client.orders();
      case 'bookstore_create_order':
        return await client.createOrder();
      case 'bookstore_checkout':
        return await client.checkout(
          args.cardNumber || '4111111111111111',
          args.expiry || '12/25',
          args.cvv || '123'
        );
      case 'bookstore_cancel_order':
        return await client.cancelOrder(args.orderId);
      case 'bookstore_create_review':
        return await client.createReview(args.bookId, args.rating, args.comment);
      case 'bookstore_delete_review':
        return await client.deleteReview(args.reviewId);
      case 'bookstore_book_reviews':
        return await client.bookReviews(args.bookId);
      case 'bookstore_my_reviews':
        return await client.myReviews();
      case 'bookstore_update_profile':
        return await client.updateProfile(args.firstName, args.lastName);
      case 'bookstore_search':
        return await client.searchBooks(args.query);
      case 'bookstore_apply_coupon':
        return await client.applyCoupon(args.code);
      case 'bookstore_register_webhook':
        return await client.registerWebhook(args.url, args.events, args.secret || '');
      case 'bookstore_webhooks':
        return await client.webhooks();
      case 'bookstore_test_webhook':
        return await client.testWebhook(args.webhookId);
      case 'bookstore_logout':
        return await client.logout();
      case 'bookstore_admin_stats':
        return await client.adminStats();
      case 'bookstore_admin_orders':
        return await client.adminOrders();
      case 'bookstore_admin_payments':
        return await client.adminPayments();
      case 'bookstore_pro_inventory':
        return await client.proInventory();
      default:
        throw new Error(`Unknown tool: ${toolName}`);
    }
  } catch (error) {
    return { error: error.message };
  }
}

class BookstoreMCPServer {
  constructor() {
    this.server = new Server(
      {
        name: 'bookstore-mcp-server',
        version: '1.0.0',
      },
      {
        capabilities: {
          tools: {},
          resources: {},
        },
      }
    );

    this.server.setRequestHandler(ListToolsRequestSchema, async () => {
      return { tools };
    });

    this.server.setRequestHandler(CallToolRequestSchema, async (request) => {
      const { name, arguments: args } = request.params;
      const result = await handleToolCall(name, args);
      return {
        content: [
          {
            type: 'text',
            text: JSON.stringify(result, null, 2),
          },
        ],
      };
    });

    this.server.setRequestHandler(ListResourcesRequestSchema, async () => {
      return { resources: [] };
    });

    this.server.setRequestHandler(ReadResourceRequestSchema, async (request) => {
      throw new Error('Resources not supported');
    });
  }

  async run() {
    const transport = new StdioServerTransport();
    await this.server.connect(transport);
    console.error('Bookstore MCP Server running on stdio');
  }
}

const server = new BookstoreMCPServer();
server.run().catch(console.error);
